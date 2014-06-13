#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <uv.h>

#include "hashtable.h"
#include "db.h"
#include "bitcase.h"
#include "cmd.h"
#include "cron.h"
#include "bcmem.h"

/*----Helper functions--*/

/*Return the UNIX microseconds time*/
long long micro_time(void) {
    struct timeval time;
    long long micro_second;

    /*Get the time and add second and microsecond*/
    gettimeofday(&time, NULL);
    micro_second = ((long long)time.tv_sec) * 1000000 + time.tv_usec;
    
    return micro_second;
}

/*Buffer allocation callback*/
uv_buf_t alloc_buffer(uv_handle_t* handle, size_t suggested_size) {
    return uv_buf_init((char*)bc_malloc(suggested_size), suggested_size);
}

/*Response write callback*/
static void after_response(uv_write_t* req, int status) {
    if (req) {
        /*Close the client stream*/
        uv_close((uv_handle_t*)req->data, NULL);
        bc_free(req);
    }
}

/*Send response to client*/
static void send_response(uv_stream_t *client, uv_buf_t *buf) {
    uv_write_t *response;

    response = (uv_write_t*)bc_malloc(sizeof(uv_write_t));
    response->data = (void*)client;

    /*Write to client stream*/
    uv_write(response, client, buf, 1, after_response);
}

/*Request read callback*/
static void after_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
    if (nread < 0) {
        /*If not EOF then error*/
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error\n");
        }
        uv_close((uv_handle_t*)stream, NULL);
    } else if (nread > 0) {
        cmd *c;
        char *tmp_buf;
        uv_buf_t response_buf;

        c = cmd_parser((server_t*)stream->data, (char*)buf.base);
        cmd_execute(c);

        /*Send response*/
        tmp_buf = (char*)bc_malloc(sizeof(c->result));
        strcpy(tmp_buf, c->result);
        cmd_free(c);
        response_buf = uv_buf_init(tmp_buf, strlen(tmp_buf));
        send_response(stream, &response_buf);
    }
    /*Release the buffer memory if used*/
    if (buf.base) {
        bc_free(buf.base);
    }
}

/*Connect callback*/
static void on_connection(uv_stream_t* stream, int status) {
    if (status == -1) {
        return;
    }
    /*Init the client stream*/
    uv_tcp_t *client = (uv_tcp_t*)bc_malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(), client);

    /*Save the server ptr from server stream to client stream*/
    client->data = stream->data;

    /*Accept the connection,start to read or close the client stream*/
    if (uv_accept(stream, (uv_stream_t*)client) == 0) {
        uv_read_start((uv_stream_t*)client, alloc_buffer, after_read);
    } else {
        uv_close((uv_handle_t*)client, NULL);
    }
}

/*Init the server*/
static void init_server(server_t *server) {
    /*Init the server loop and server stream*/
    server->loop = uv_default_loop();
    server->stream = (uv_tcp_t*)bc_malloc(sizeof(uv_tcp_t));
    /*Save the server ptr in the stream*/
    server->stream->data = (void*)server;
    if (uv_tcp_init(server->loop, server->stream)) {
        fprintf(stderr, "Socket create error\n");
        return;
    }

    /*Bind the addr to server*/
    server->bind_addr = uv_ip4_addr("0.0.0.0", BC_PORT);
    if (uv_tcp_bind(server->stream, server->bind_addr)) {
        fprintf(stderr, "Addr bind error\n");
        return;
    }

    /*Init DB and command table*/
    server->d = (db*)bc_malloc(sizeof(db));
    db_init(server->d);

    server->commands = (hash_table*)bc_malloc(sizeof(hash_table));
    if (cmd_init_commands(server->commands) == CMD_ERROR) {
        fprintf(stderr, "Command table init error");
        return;
    }
}

/*Start cron jobs*/
static void start_cron(server_t *server) {
    uv_timer_t *resize_timer;
    uv_timer_t *rehash_timer;
    uv_timer_t *mem_count_timer;

    /*Malloc memory*/
    resize_timer = (uv_timer_t*)bc_malloc(sizeof(uv_timer_t));
    rehash_timer = (uv_timer_t*)bc_malloc(sizeof(uv_timer_t));
    mem_count_timer = (uv_timer_t*)bc_malloc(sizeof(uv_timer_t));

    /*Save the server ptr in the timer handle*/
    resize_timer->data = (void*)server;
    rehash_timer->data = (void*)server;
    mem_count_timer->data = (void*)server;

    /*Init these timers*/
    uv_timer_init(server->loop, resize_timer);
    uv_timer_init(server->loop, rehash_timer);
    uv_timer_init(server->loop, mem_count_timer);

    /*Start these timers*/
    uv_timer_start(resize_timer, cron_check_resize, 5000, RESIZE_INTERVAL);
    uv_timer_start(rehash_timer, cron_rehash, 5000, REHASH_INTERVAL);
    uv_timer_start(mem_count_timer, cron_count_memory, 5000,
                       MEM_COUNT_INTERVAL);

}


int main(int argc, char **argv) {
    int errno;
    server_t *server;

    server = (server_t*)bc_malloc(sizeof(server_t));
    /*Init server and start cron jobs*/
    init_server(server);
    start_cron(server);

    /*Server start to listen connections*/
    errno = uv_listen((uv_stream_t*)server->stream, 128, on_connection);
    if (errno) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(errno));
        return BC_ERROR;
    }

    uv_run(server->loop, UV_RUN_DEFAULT);

    return 0;
}
