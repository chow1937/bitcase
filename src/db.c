#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <uv.h>

#include "hashtable.h"
#include "db.h"
#include "lru.h"
#include "bitcase.h"

/*Init a db*/
void db_init(db *d) {
    d->ht[0] = ht_create();
    d->ht[1] = NULL;

    if (d->ht[0]) {
        /*Set init value*/
        d->is_rehash = 0;
        d->rehash_index = 0;
        d->mem_limit = DB_MAX_MEM;
        d->llist = (lru_list*)malloc(sizeof(lru_list));
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
        fprintf(stderr, "DB create error,not enouth memory\n");
        return NULL;
    }
}

/*Do n step rehash,use db->ht[1] as the temp hash table*/
int db_rehash(db *d, int n) {
    bucket *ptr_bk, *next;
    uint32_t hash, index;

    if (!d->is_rehash) {
        fprintf(stderr, "DB is not in rehashing now\n");
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

/*Rehash the db in n milliseconds, each time try 100 step*/
int db_rehash_millisec(db *d, int n) {
    long long start = micro_time();

    while(micro_time() - start < n * 1000) {
        if (db_rehash(d, 100) == DB_ERROR) {
            fprintf(stderr, "Rehash error\n");
            return DB_ERROR;
        }
    }

    return DB_OK;
}

/*Reduce or expand the db*/
int db_resize(db *d, uint32_t size) {
    int rv;
    hash_table *ht;

    rv = ht_alloc(ht, size);
    if (rv == HT_OK) {
        d->ht[1] = ht;
        d->is_rehash = 1;

        return DB_OK;
    }

    fprintf(stderr, "Hash table alloc error\n");
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
        bk = ht_find(d->ht[0], key);
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
    char *bk_key, *bk_value;

    /*Alloc memory,these are the really used*/
    bk_key = (char*)malloc(sizeof(key));
    bk_value = (char*)malloc(sizeof(value));
    /*Copy the command argv*/
    strcpy(bk_key, key);
    strcpy(bk_value, value);

    /*Choose the correct hashtable*/
    if (d->is_rehash) {
        ht = d->ht[1];
    } else {
        ht = d->ht[0];
    }

    /*Add the key to the hash table and attach the LRU node*/
    bk = ht_add(ht, bk_key, bk_value);
    if (bk) {
        bk->lru = (lru_node*)malloc(sizeof(lru_node));
        if (bk->lru) {
            /*Attach the bucket's lru node to lru list*/
            bk->lru->bk = bk;
            attach(d->llist, bk->lru);
        } else {
            fprintf(stderr, "Alloc memory for lru node error\n");
            return DB_ERROR;
        }
    } else {
        return DB_ERROR;
    }

    return DB_OK;
}

/*Update a key's value,the key should be in the DB before*/
int db_update_key(db *d, void *key, void *value) {
    bucket *bk;
    hash_table *ht;
    char *bk_value;

    /*Alloc memory for value,he really used*/
    bk_value = (char*)malloc(sizeof(value));
    /*Copy the command argv*/
    strcpy(bk_value, value);

    /*Choose the correct hashtable*/
    if (d->is_rehash) {
        ht = d->ht[1];
    } else {
        ht = d->ht[0];
    }

    bk = ht_update(ht, key, bk_value);
    if (bk) {
        detach(bk->lru);
        attach(d->llist, bk->lru);

        return DB_OK;
    }

    return DB_ERROR;
}

/*Delete a bucket by key*/
int db_delete_key(db *d, void *key) {
    int i, rv;

    /*Choose the correct hashtable*/
    if (d->is_rehash) {
        for(i = 0;i < 2;i++) {
            rv = ht_delete(d->ht[i], key);
            if (rv==HT_OK) break;
        }
    } else {
        rv = ht_delete(d->ht[0], key);
    }

    if (rv == HT_OK) {
        return DB_OK;
    }
    return DB_ERROR;
}

/*Count a DB's used memory,count the key/value only*/
uint32_t db_count_mem(db *d) {
    int i, j;
    uint32_t used = 0;
    bucket *head, *next;
    hash_table *ht;

    if (d->is_rehash) {
        /*If the DB is rehashing,count both ht[0] and ht[1]*/
        for (i = 0;i < 2;i++) {
            ht = d->ht[i];
            for (j = 0;j < ht->size;j++) {
                head = ht->table[j];
                /*Iter the bucket list*/
                while (head) {
                    next = head->next;
                    used += strlen(head->key);
                    used += strlen(head->value);

                    head = next;
                }
            }
        }
    } else {
        /*Count the ht[0] only*/
        ht = d->ht[0];
        for (j = 0;j < ht->size;j++) {
            head = ht->table[j];
            /*Iter the bucket list*/
            while (head) {
                next = head->next;
                used += strlen(head->key);
                used += strlen(head->value);

                head = next;
            }
        }
    }

    return used;
}
