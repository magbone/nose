

#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "nose.h"
#include "config.h"
#include "device.h"

#include <uv.h>

static uv_loop_t *loop;
static uv_tcp_t client;
static uv_poll_t *utun_poll;
static struct config _conf;

static void on_connect(uv_connect_t* req, int status);
static void utun_poll_cb(uv_poll_t *handle, int status, int event);

int client_loop(struct config conf) ;

#endif // !_CLIENT_H_