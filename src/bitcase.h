#ifndef _BITCASE_H
#define _BITCASE_H

/*Server configure*/
#define BC_PORT 8638

/*Error codes*/
#define BC_OK 0
#define BC_ERROR -1

/*Callbacks*/
uv_buf_t alloc_buffer(uv_handle_t* handle, size_t suggested_size);
void on_connection(uv_stream_t* server, int status);
void after_read(uv_stream_t* stream, ssize_t nread, uv_buf_t buf);
void after_response(uv_write_t* req, int status);

#endif
