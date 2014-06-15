#ifndef _DB_H
#define _DB_H

#include <stdint.h>

/*Status code*/
#define DB_OK 0
#define DB_ERROR -1

/*2G memory*/
#define DB_MAX_MEM 2147483648
#define DB_REDUCE_RATIO 10
#define DB_EXPEND_RATIO 2

/*db struct*/
typedef struct db {
    struct hash_table *ht[2];
    int is_rehash;
    uint32_t rehash_index;
    /*DB memory limit*/
    uint32_t mem_limit;
    /*Double linked list, for LRU*/
    struct lru_list *llist;
} db;

/*APIs*/
void db_init(db *d);
db *db_create(void);
bucket *db_get_key(db *d, void *key);
int db_add_key(db *d, void *key, void *value);
int db_update_key(db *d, void *key, void *value);
int db_delete_key(db *d, void *key);
int db_rehash(db *d, int n);
int db_rehash_millisec(db *d, int n);
int db_resize(db *d, uint32_t size);
uint32_t db_count_mem(db *d);

#endif
