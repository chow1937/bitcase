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

/*Reset the hashtable*/
void ht_reset(hash_table *ht) {
    ht->table = NULL;
    ht->mask = 0;
    ht->size = 0;
    ht->used = 0;
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
            /*Free the key, value and the bucket*/
            free(head->key);
            free(head->value);
            free(head);

            ht->used--;
            head = next;
        }
    }

    free(ht->table);
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
    ht->table[index] = new;
    ht->used++;

    return HT_OK;
}
