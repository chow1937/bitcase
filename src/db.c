#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "db.h"
#include "hashtable.h"

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
        d->used_mem = 0;
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
            /*When finish rehash,move LRU list to the used hashtable*/
            d->ht[1]->llist = d->ht[0]->llist;
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
