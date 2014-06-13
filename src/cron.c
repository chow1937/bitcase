#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "hashtable.h"
#include "db.h"
#include "bitcase.h"
#include "lru.h"
#include "cron.h"

/*
 * Check if the db need resize.
 *
 * If ht used/size < 10%,then reduce the ht size to used.
 * If ht used/size > 2, then expand the ht size to db_next_size(used)
 * */
void cron_check_resize(uv_timer_t* handle, int status) {
    server_t *server = (server_t*)handle->data;

    /*If already started rehash,no need to check*/
    if (server->d->is_rehash) return;

    unsigned long used, size;

    used = server->d->ht[0]->used;
    size = server->d->ht[0]->size;

    /*If just start,don't resize*/
    if (used == 0) return;

    /*If the radio less than %10,reduce the hashtable*/
    if (100 * used / size < DB_REDUCE_RATIO) {
        db_resize(server->d, used);
        return;
    }
    /*If the radio more than 2, expand the hashtable*/
    if (used / size > DB_EXPEND_RATIO) {
        db_resize(server->d, ht_next_size(size));
        return;
    }
}

/*Rehash the DB that already start to rehash for 1 millisecond*/
void cron_rehash(uv_timer_t* handle, int status) {
    server_t *server = (server_t*)handle->data;

    /*If the DB is not rehashing now,return*/
    if (!server->d->is_rehash) return;
    db_rehash_millisec(server->d, 1);
}

/*Count the memory,if over limit then start the LRU*/
void cron_count_memory(uv_timer_t* handle, int status) {
    int i;
    server_t *server = (server_t*)handle->data;
    uint32_t mem_used = db_count_mem(server->d);

    if (mem_used > server->d->mem_limit) {
        /*Remove 100 bucket from the tail of the LRU lis*/
        lru_remove(server->d, 100);
    }
}
