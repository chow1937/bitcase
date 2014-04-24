#ifndef _CRON_H
#define _CRON_H

/*Cron jobs define as libuv timer callback*/
void cron_check_resize(uv_timer_t* handle, int status);
void cron_rehash(uv_timer_t* handle, int status);
void cron_count_memory(uv_timer_t* handle, int status);

#endif
