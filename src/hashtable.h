#ifndef _HASHTABLE_H
#define _HASHTABLE_H

/*Status codes*/
#define HT_OK 0
#define HT_ERROR -1

/*HashTable minimum size*/
#define HT_MIN_SIZE 4
/*Hash function seed*/
#define HT_SEED 0x51203344

/*HashTable node bucket*/
typedef struct bucket {
    void *key;
    void *value;
    /*next pointer to deal with the collision*/
    struct bucket *next;
} bucket;

/*HashTable struct*/
typedef struct hash_table {
    /*hashtable bucket pointer array*/
    bucket **table;
    /*size mask,to count the array index*/
    unsigned long mask;
    unsigned long size;
    unsigned long used;
} hash_table;

/*APIs*/
hash_table *ht_create(void);
bucket *ht_find(hash_table *ht, void *key);
void ht_reset(hash_table *ht);
int ht_clear(hash_table *ht);
int ht_alloc(hash_table *ht, unsigned long size);
int ht_add(hash_table *ht, void *key, void *value);
int ht_delete(hash_table *ht, void *key);
int ht_update(hash_table *ht, void *key, void *value);
int ht_free_bucket(bucket *bc);
uint32_t gen_hash(const void *key);

#endif
