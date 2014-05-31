#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "hashtable.h"
#include "db.h"
#include "lru.h"
#include "bitcase.h"
#include "bcmem.h"

/*Init a lru list*/
int lru_init(lru_list *llist) {
    if (llist) {
        llist->head = (lru_node*)bc_malloc(sizeof(lru_node));
        llist->tail = (lru_node*)bc_malloc(sizeof(lru_node));
        if (llist->head && llist->tail) {
            /*Init head and tail node*/
            llist->head->prev = NULL;
            llist->head->next = llist->tail;
            llist->tail->prev = llist->head;
            llist->tail->next = NULL;

            /*Set the head and tail node bk to NULL*/
            llist->head->bk = NULL;
            llist->tail->bk = NULL;

            return LRU_OK;
        }
        return LRU_ERROR;
    }

    return LRU_ERROR;
}

/*Attache a lru_node to a lru_list head*/
int attach(lru_list *llist, lru_node *lru) {
    /*Set the new node first*/
    lru->prev = llist->head;
    lru->next = llist->head->next;
    /*Disconnect head and next*/
    llist->head->next = lru;
    lru->next->prev = lru;

    return LRU_OK;
}

/*Detach a lru from a lru_list*/
int detach(lru_node *lru) {
    lru->prev->next = lru->next;
    lru->next->prev = lru->prev;

    return LRU_OK;
}

/*Detach and remove n node from tail*/
int lru_remove(lru_list *llist, int n) {
    int i;
    lru_node *node;

    for (i = 0;i < 100;i++) {
        node = llist->tail->prev;
        detach(node);
        db_delete_key(server.d, node->bk->key);
    }

    return LRU_OK;
}
