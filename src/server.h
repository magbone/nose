

#ifndef _SERVER_H_
#define _SERVER_H_

#include "nose.h"
#include "config.h"
#include "common.h"

#include <uv.h>


static uv_loop_t *loop;
static int connection_count = 0;

static uv_tcp_t *clients[2] = {NULL};

static void add_client(uv_tcp_t *client);
static void remove_client(uv_tcp_t *client);
static uv_tcp_t * get_other_client(uv_tcp_t *client);


void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void on_close(uv_handle_t *handle);
void free_write_req(uv_write_t *req);
static void forwarding_write(uv_write_t *req, int status);
static void forwarding_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
static void on_new_connection(uv_stream_t *server, int status);

int server_loop(struct config conf);

#endif // !_SERVER_H_