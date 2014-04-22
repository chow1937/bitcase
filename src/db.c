#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hashtable.h"
#include "db.h"
#include "lru.h"

/*Init a db*/
void db_init(db *d) {
    int i;

    for (i = 0;i < 2;i++) {
        d->ht[i] = ht_create();
    }
    if (d->ht[0] && d->ht[1]) {
        /*Set init value*/
        d->is_rehash = 0;
        d->rehash_index = 0;
        d->mem_limit = DB_MAX_MEM;
        /*Init the LRU list*/
        lru_init(d->llist);
    } else {
        fprintf(stderr, "DB init error");
    }
}

/*Create a db*/
db *db_create(void) {
    db *d;

    d = (db*)malloc(sizeof(db));
    if (d) {
        db_init(d);
        return d;
    } else {
        fprintf(stderr, "DB create error,not enouth memory");
        return NULL;
    }
}

/*Do n step rehash,use db->ht[1] as the temp hash table*/
int db_rehash(db *d, int n) {
    bucket *ptr_bk, *next;
    uint32_t hash, index;

    if (!d->is_rehash) {
        fprintf(stderr, "DB is not in rehashing now");
        return DB_ERROR;
    }

    while (n--) {
        if (d->ht[0]->used == 0) {
            free(d->ht[0]->table);
            d->ht[0] = d->ht[1];
            ht_reset(d->ht[1]);
            d->is_rehash = 0;

            return DB_OK;
        } else {
            /*Find the next not NULL slot of the ht[0]*/
            while (d->ht[0]->table[d->rehash_index] == NULL) {
                d->rehash_index++;
            }
            /*Continue the rehash process*/
            ptr_bk = d->ht[0]->table[d->rehash_index];
            while (ptr_bk) {
                next = ptr_bk->next;
                /*Count hash and index*/
                hash = ht_gen_hash(ptr_bk->key);
                index = hash & d->ht[1]->mask; 

                ptr_bk->next = d->ht[1]->table[index];
                d->ht[1]->table[index] = ptr_bk;

                /*Update counter*/
                d->ht[0]--;
                d->ht[1]++;

                ptr_bk = next;
            }
        }
        d->ht[0]->table[d->rehash_index] = NULL;
        d->rehash_index++;
    }

    return DB_OK;
}

/*Reduce or expand the db*/
int db_resize(db *d, uint32_t size) {
    int rv;
    hash_table *ht;

    rv = ht_alloc(ht, size);
    if (rv == 0) {
        d->ht[1] = ht;
        d->is_rehash = 1;

        return DB_OK;
    }

    fprintf(stderr, "Hash table alloc error");
    return DB_ERROR;
}

/*Get a bucket by key*/
bucket *db_get_key(db *d, void *key) {
    int i;
    bucket *bk;

    if (d->is_rehash) {
        /*If the DB already start rehash,search both the two hashtable*/
        for (i = 0;i < 2;i++) {
            bk = ht_find(d->ht[i], key);
            if (bk) break;
        }
    } else {
        bk = ht_find(d->ht[i], key);
    }

    if (bk) {
        /*Update the bucket LRU status*/
        detach(bk->lru);
        attach(d->llist, bk->lru);
    }
    return bk;
}

/*Add a bucket to the DB*/
int db_add_key(db *d, void *key, void *value) {
    bucket *bk;
    hash_table *ht;

    /*Choose the correct hashtable*/
    if (d->is_rehash) {
        ht = d->ht[1];
    } else {
        ht = d->ht[0];
    }

    /*Add the key to the hash table and attach the LRU node*/
    bk = ht_add(ht, key, value);
    bk->lru = (lru_node*)malloc(sizeof(lru_node));
    if (bk->lru) {
        /*Attach the bucket's lru node to lru list*/
        bk->lru->bk = bk;
        attach(d->llist, bk->lru);
    } else {
        fprintf(stderr, "Alloc memory for lru node error");
        return DB_ERROR;
    }

    return DB_OK;
}

/*Update a key's value,the key should be in the DB before*/
int db_update_key(db *d, void *key, void *value) {
    bucket *bk;
    hash_table *ht;
    uint32_t mem_len, old_mem_len;

    /*Choose the correct hashtable*/
    if (d->is_rehash) {
        ht = d->ht[1];
    } else {
        ht = d->ht[0];
    }

    bk = ht_update(ht, key, value);
    if (bk) {
        detach(bk->lru);
        attach(d->llist, bk->lru);

        return DB_OK;
    } else {
        /*If key not found in the DB,add the kye/value to the DB*/
        db_add_key(d, key, value);
    }
}

/*Delete a bucket by key*/
int db_delete_key(db *d, void *key) {
    int i, rv;
    hash_table *ht;

    /*Choose the correct hashtable*/
    if (d->is_rehash) {
        ht = d->ht[1];
    } else {
        ht = d->ht[0];
    }

    rv = ht_delete(ht, key);
    if (rv == HT_OK) {
        return DB_OK;
    }
    return DB_ERROR;
}
