#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hashtable.h"

/*----Hash function and it's helper functions----*/

/*Rotate a 32 bit unsigned integer left*/
inline uint32_t rotl(uint32_t x, int8_t r) {
    return (x << r) | (x >> (32 - r));
}

/*Finalization mix - force all bits of a hash block to avalanche*/
inline uint32_t fmix(uint32_t h) {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

/*
 * MurmurHash3, by Austin Appleby
 *
 * More infomation, please refer to https://code.google.com/p/smhasher/
 */
uint32_t MurmurHash3(const void *key, int len, uint32_t seed) {
    const uint8_t * data = (const uint8_t*)key;
    const int nblocks = len / 4;

    uint32_t h1 = seed;

    uint32_t c1 = 0xcc9e2d51;
    uint32_t c2 = 0x1b873593;

    //----------
    // body

    const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

    for(int i = -nblocks; i; i++) {
        uint32_t k1 = blocks[i];

        k1 *= c1;
        k1 = rotl(k1,15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl(h1,13); 
        h1 = h1*5+0xe6546b64;
    }

    //----------
    // tail

    const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

    uint32_t k1 = 0;

    switch(len & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1; k1 = rotl(k1,15); k1 *= c2; h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= len;
    h1 = fmix(h1);

    return h1;
}


/*----API implementations----*/

/*Generate hash*/
uint32_t gen_hash(const void *key) {
    uint32_t seed = HT_SEED;
    uint32_t h1;
    size_t len;

    len = strlen((char*)key);

    /*Invoke the real hash function*/
    h1 = MurmurHash3(key, len, seed);

    return h1;
}

/*Init a lru list*/
int ht_init_lru(lru_list *llist) {
    llist = (lru_list*)malloc(sizeof(lru_list));
    if (llist) {
        llist->head = (lru_node*)malloc(sizeof(lru_node));
        llist->tail = (lru_node*)malloc(sizeof(lru_node));
        if (llist->head && llist->tail) {
            /*Init head and tail node*/
            llist->head->prev = NULL;
            llist->head->next = llist->tail;
            llist->tail->prev = llist->head;
            llist->tail->next = NULL;

            return HT_OK;
        }
        return HT_ERROR;
    }

    return HT_ERROR;
}

/*Attache a lru_node to a lru_list head*/
int ht_attach_lru(lru_list *llist, lru_node *lru) {
    /*Set the new node first*/
    lru->prev = llist->head;
    lru->next = llist->head->next;
    /*Disconnect head and next*/
    llist->head->next = lru;
    lru->next->prev = lru;

    return HT_OK;
}

/*Detache a lru from a lru_list*/
int ht_detach_lru(lru_node *lru) {
    lru->prev->next = lru->next;
    lru->next->prev = lru->prev;

    return HT_OK;
}

/*Free a bucket*/
int ht_free_bucket(bucket *bk) {
    free(bk->key);
    free(bk->value);
    /*
     * When free a bucket ,detach the lru node from the lru list
     * */
    ht_detach_lru(bk->lru);
    free(bk->lru);
    free(bk);

    return HT_OK;
}

/*Reset the hashtable*/
void ht_reset(hash_table *ht) {
    ht->table = NULL;
    ht->mask = 0;
    ht->size = 0;
    ht->used = 0;
    ht->llist = NULL;
}

/*Clear the hashtable and release the memory*/
int ht_clear(hash_table *ht) {
    int i;
    bucket *head, *next;

    /*Iter the whole hash table and release the memory*/
    for (i = 0; i < ht->size; i++) {
        head = ht->table[i];
        /*If the bucket has nothing,then continue*/
        if (head == NULL) continue;
        /*Else release all elements link by the bucket*/
        while (head) {
            next = head->next;
            ht_free_bucket(head);

            ht->used--;
            head = next;
        }
    }

    free(ht->table);
    free(ht->llist);
    ht_reset(ht);

    return HT_OK;
}

/*Alloc memory for hashtable*/
int ht_alloc(hash_table *ht, unsigned long size) {
    unsigned long real_size;
    /*Make sure the hashtable size not smaller than HT_MIN_SIZE*/
    if (size < HT_MIN_SIZE) {
        real_size = HT_MIN_SIZE;
    } else {
        real_size = size;
    }

    /*Init the hashtable*/
    ht->size = real_size;
    ht->mask = real_size - 1;
    ht->used = 0;
    ht_init_lru(ht->llist);

    /*Alloc memory*/
    ht->table = (bucket**)malloc(real_size*sizeof(bucket*));
    memset(ht->table, '\0', real_size);

    if (ht->table) {
        return HT_OK; 
    } else {
        fprintf(stderr, "Memory malloc error\n");
        return HT_ERROR;
    }
}

/*Create a new hashtable*/
hash_table *ht_create(void) {
    /*Alloc memory*/
    hash_table *ht = (hash_table*)malloc(sizeof(hash_table));
    memset(ht, '\0', sizeof(hash_table));
    if (ht == NULL) {
        fprintf(stderr, "Memory malloc error\n");
    }
    ht_alloc(ht, HT_MIN_SIZE);

    return ht;
}

/*Add a new key/value into a hashtable*/
int ht_add(hash_table *ht, void *key, void *value) {
    uint32_t i, index, hash;
    bucket *head, *next, *new;

    /*Generate hash and count table index*/
    hash = gen_hash(key);
    index = hash & ht->mask;

    /*Try to insert the new key/value into a free slot*/
    head = ht->table[index];
    if (head) {
        bucket *tmp;
        tmp = head;
        /*Iter the bucket list*/
        while (head) {
            next = head->next;
            /*If the key already exists,then return HT_ERROR*/
            if (strcmp((char*)key, (char*)head->key) == 0) {
                return HT_ERROR;
            }
            head = next;
        }
        /*Asign the head bucket address back to var head*/
        head = tmp;
    }

    new = (bucket*)malloc(sizeof(bucket));
    new->next = head;
    /*Apply key and value to the new bucket*/
    new->key = key;
    new->value = value;

    /*Deal with the LRU thing*/
    new->lru = (lru_node*)malloc(sizeof(lru_node));
    new->lru->bk = new;
    /*
     * When add a new bucket,attach a new lru node to the
     * head of the lru list.
     * */
    ht_attach_lru(ht->llist, new->lru);

    ht->table[index] = new;
    ht->used++;

    return HT_OK;
}

/*Delete a key/value pair from hashtable by key*/
int ht_delete(hash_table *ht, void *key) {
    uint32_t hash, index;
    bucket *prev, *ptr, *next;

    /*Generate hash and count index*/
    hash = gen_hash(key);
    index = hash & ht->mask;

    prev = NULL;
    ptr = ht->table[index];
    if (ptr == NULL) {
        /*Key not exists in this hashtable,return HT_ERROR*/
        fprintf(stderr, "Delete error,%s not exists", (char*)key);
        return HT_ERROR;
    }

    /*Iter the bucket list*/
    while (ptr) {
        next = ptr->next;
        if (strcmp((char*)key, (char*)ptr->key) == 0) {
            ht_free_bucket(next);
            ht->used--;
            if (prev == NULL) {
                ht->table[index] = next;
            } else {
                prev->next = next;
            }

            return HT_OK;
        }
        /*Move forward*/
        prev = ptr;
        ptr = next;
    }

    return HT_ERROR;
}

/*Find a bucket in the hashtable by key*/
bucket *ht_find(hash_table *ht, void *key) {
    uint32_t hash, index;
    bucket *head, *next;

    hash = gen_hash(key);
    index = hash & ht->mask;
    head = ht->table[index];

    /*Iter the bucket list*/
    while(head) {
        next = head->next;
        if(strcmp((char*)key, (char*)head->key) == 0)  {
            return head;
        }
        head = next;
    }

    return NULL;
}

/*Update a bucket by key*/
int ht_update(hash_table *ht, void *key, void *value) {
    bucket *bk;

    bk = ht_find(ht, key);
    if (bk) {
        bk->value = value;
        /*
         * When find a key, first detach the lru node,
         * then attach the lru node to the head of the
         * lru list.
         * */
        ht_detach_lru(bk->lru);
        ht_attach_lru(ht->llist, bk->lru);

        return HT_OK;
    } else {
        fprintf(stderr, "Update error, key %s not exists", (char*)key);
        return HT_ERROR;
    }
}
