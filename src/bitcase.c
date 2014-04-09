#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "bitcase.h"

/*Buffer allocation callback*/
uv_buf_t alloc_buffer(uv_handle_t* handle, size_t suggested_size) {
    return uv_buf_init((char*)malloc(suggested_size), suggested_size);
}

/*Connect callback*/
void on_connection(uv_stream_t* server, int status) {
    if (status == -1) {
        return;
    }
    /*Init the client stream*/
    uv_tcp_t *client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    uv_tcp_init(uv_default_loop(), client);
    /*Accept the connection,start to read or close the client stream*/
    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        uv_read_start((uv_stream_t*)client, alloc_buffer, after_read);
    } else {
        uv_close((uv_handle_t*)client, NULL);
    }
}

/*Request read callback*/
void after_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf) {
    if (nread < 0) {
        /*If not EOF then error*/
        if (nread != UV_EOF) {
            fprintf(stderr, "Read error\n");
        }
        uv_close((uv_handle_t*)stream, NULL);
    } else if (nread > 0) {
    
    }
    /*Release the buffer memory if used*/
    if (buf.base) {
        free(buf.base);
    }
}

/*Response write callback*/
void after_response(uv_write_t* req, int status) {

}


int main(int argc, char **argv) {
    int errno;
    uv_loop_t *loop = uv_default_loop();
    uv_tcp_t server;
    struct sockaddr_in bind_addr = uv_ip4_addr("0.0.0.0", BC_PORT);
    /*Init the server loop and server stream*/
    if (uv_tcp_init(loop, &server)) {
        fprintf(stderr, "Socket create error\n");
        return BC_ERROR;
    }
    /*Bind the addr to server*/
    if (uv_tcp_bind(&server, bind_addr)) {
        fprintf(stderr, "Addr bind error\n");
    }
    /*Server start to listen connections*/
    errno = uv_listen((uv_stream_t*)&server, 128, on_connection);
    if (errno) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(errno));
        return BC_ERROR;
    }

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}
