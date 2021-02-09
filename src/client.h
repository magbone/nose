

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "nose.h"
#include "config.h"
#include "device.h"
#include "common.h"
#include "dfa.h"

#include <uv.h>

static uv_loop_t *loop;
static uv_tcp_t *client;
static struct config _conf;
static dfa* dfa_handler;
static int timer_id;


extern void free_write_req(uv_write_t *req);
extern void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);

static void read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);
static void client_write(uv_write_t *req, int status);
static void on_close(uv_handle_t *handle);
static void on_connect(uv_connect_t* req, int status);
static void * utun_read_process(void* args);
static void dfa_cb(int before, int after, int condition, void* arg);
static void * send_auth_req_pkt(void* args);
static int pkt_unpack(char *buffer, int len);

int client_loop(struct config conf) ;

#endif // !_CLIENT_H_