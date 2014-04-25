#ifndef _CRON_H
#define _CRON_H

/*Cron jobs intervals*/
#define RESIZE_INTERVAL 500
#define REHASH_INTERVAL 200
#define MEM_COUNT_INTERVAL 2000

/*Cron jobs define as libuv timer callback*/
void cron_check_resize(uv_timer_t* handle, int status);
void cron_rehash(uv_timer_t* handle, int status);
void cron_count_memory(uv_timer_t* handle, int status);

#endif
