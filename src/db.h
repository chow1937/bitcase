#ifndef _DB_H
#define _DB_H

/*Status code*/
#define DB_OK 0
#define DB_ERROR -1

#define DB_MAX_MEM 2147483648

/*db struct*/
typedef struct db {
    struct hash_table *ht[2];
    int is_rehash;
    uint32_t rehash_index;
    /*DB memory limit*/
    uint32_t mem_limit;
    /*Count the key and value only*/
    uint32_t used_mem;
} db;

/*APIs*/
void db_init(db *d);
db *db_create(void);
bucket *db_get_key(db *d, void *key);
int db_set_key(db *d, void *key, void *value);
int db_delete_key(db *d, void *key);
int db_rehash(db *d, int n);
int db_resize(db *d, uint32_t size);

#endif
