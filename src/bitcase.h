#ifndef _BITCASE_H
#define _BITCASE_H

/*Status codes*/
#define BC_OK 0
#define BC_ERROR -1

/*Server configure*/
#define BC_PORT 8638

typedef struct server {
    /*Server event loop*/
    uv_loop_t *loop;
    /*Server stream*/
    uv_tcp_t *stream;
    /*Server addr*/
    struct sockaddr_in bind_addr;
    /*DB that used*/
    db *d;
    /*Command hashtable*/
    hash_table *commands;
} server_t;

/*----Extern variables---*/
extern server_t server;

/*Helper functions*/
long long micro_time(void);

#endif
