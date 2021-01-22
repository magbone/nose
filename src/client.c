

#include "client.h"

static void 
utun_poll_cb(uv_poll_t *handle, int status, int event)
{
      if(status < 0){
            fprintf(stderr, "[ERROR] Read error %s\n", uv_strerror(status));
            return;
      }

      switch (event)
      {
      case UV_READABLE:
            
            break;
      case UV_WRITABLE:
            break;
      default:
            break;
      }
}

static void 
on_connect(uv_connect_t* req, int status)
{
      if(status < 0)
      {
            fprintf(stderr, "[ERROR] Connection error %s\n", uv_strerror(status));
            return;
      }
      fprintf(stdout, "[INFO] Connection to remote server\n");

      utun_poll = (uv_poll_t *)malloc(sizeof(uv_poll_t));
      uv_poll_init(loop, utun_poll, _conf.utun_fd);

      uv_poll_start(utun_poll, UV_READABLE, utun_poll_cb);
}

int client_loop(struct config conf)
{
      loop = uv_default_loop();
      _conf = conf;

      uv_tcp_init(loop, &client);
      struct sockaddr_in addr;
      uv_connect_t *connect = (uv_connect_t *)malloc(sizeof(uv_connect_t));

      uv_ip4_addr(_conf.server_host, _conf.server_port, &addr);

      int ret = uv_tcp_connect(connect, &client, (const struct sockaddr *)&addr, on_connect);

      if(ret)
      {
            fprintf(stderr, "[ERROR] Connect error %s\n", uv_strerror(ret));
            return 1;
      }
      return uv_run(loop, UV_RUN_DEFAULT);
}