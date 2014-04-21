#ifndef _HASHTABLE_H
#define _HASHTABLE_H

/*Status codes*/
#define HT_OK 0
#define HT_ERROR -1

/*HashTable minimum size*/
#define HT_MIN_SIZE 4
/*Hash function seed*/
#define HT_SEED 0x51203344

/*Node for lru double linked list*/
typedef struct lru_node {
    struct lru_node *prev;
    struct lru_node *next;
    /*Pointer to the bucket*/
    struct bucket *bk;
} lru_node;

/*The lru double linked list*/
typedef struct lru_list {
    lru_node *head;
    lru_node *tail;
} lru_list;

/*HashTable node bucket*/
typedef struct bucket {
    void *key;
    void *value;
    /*next pointer to deal with the collision*/
    struct bucket *next;
    /*LRU list node*/
    lru_node *lru;
} bucket;

/*HashTable struct*/
typedef struct hash_table {
    /*Hashtable bucket pointer array*/
    bucket **table;
    /*Size mask,to count the array index*/
    unsigned long mask;
    unsigned long size;
    unsigned long used;
    /*Double linked list, for LRU*/
    lru_list *llist;
} hash_table;

/*APIs*/
int ht_init_lru(lru_list *llist);
int ht_attach_lru(lru_list *llist, lru_node *lru);
int ht_detach_lru(lru_node *lru);
int ht_free_bucket(bucket *bk);
hash_table *ht_create(void);
bucket *ht_find(hash_table *ht, void *key);
void ht_reset(hash_table *ht);
int ht_clear(hash_table *ht);
int ht_alloc(hash_table *ht, unsigned long size);
int ht_add(hash_table *ht, void *key, void *value);
int ht_delete(hash_table *ht, void *key);
int ht_update(hash_table *ht, void *key, void *value);
uint32_t ht_gen_hash(const void *key);

#endif
