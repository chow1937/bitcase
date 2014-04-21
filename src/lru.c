#include <stdio.h>
#include <stdlib.h>

#include "lru.h"

/*Init a lru list*/
int init_lru(lru_list *llist) {
    llist = (lru_list*)malloc(sizeof(lru_list));
    if (llist) {
        llist->head = (lru_node*)malloc(sizeof(lru_node));
        llist->tail = (lru_node*)malloc(sizeof(lru_node));
        if (llist->head && llist->tail) {
            /*Init head and tail node*/
            llist->head->prev = NULL;
            llist->head->next = llist->tail;
            llist->tail->prev = llist->head;
            llist->tail->next = NULL;

            return HT_OK;
        }
        return HT_ERROR;
    }

    return HT_ERROR;
}

/*Attache a lru_node to a lru_list head*/
int attach(lru_list *llist, lru_node *lru) {
    /*Set the new node first*/
    lru->prev = llist->head;
    lru->next = llist->head->next;
    /*Disconnect head and next*/
    llist->head->next = lru;
    lru->next->prev = lru;

    return HT_OK;
}

/*Detache a lru from a lru_list*/
int detach(lru_node *lru) {
    lru->prev->next = lru->next;
    lru->next->prev = lru->prev;

    return HT_OK;
}