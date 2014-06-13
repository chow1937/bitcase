#ifndef _LRU_H
#define _LRU_H

#define 	LRU_OK 0
#define LRU_ERROR -1

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

/*APIs*/
int lru_init(lru_list *llist);
int attach(lru_list *llist, lru_node *node);
int detach(lru_node *node);
int lru_remove(db *d, int n);

#endif
