

#include "client.h"
#include "device.h"
#include "utils.h"
#include "icmp.h"

#include <unistd.h>
#include <pthread.h>

static void 
on_close(uv_handle_t *handle)
{
      if (handle != NULL)
            free(handle);
}

static void 
read_cb(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
      if(nread > 0)
      {
            fprintf(stdout, "[INFO] Receive data length: %ld\n", nread);

            char buffer[nread];
            memcpy(buffer, buf->base, nread);
            utun_write(_conf.utun_fd, buffer, nread);
            return;
            
      }
      else if(nread < 0)
      {
            if(nread != UV_EOF)
                  fprintf(stderr, "[ERROR] Read error: %s\n", uv_err_name(nread));
            else
                  fprintf(stderr, "[ERROR] Client disconnect\n");

            uv_close((uv_handle_t *)stream, on_close);
      }

      if (buf->base != NULL) free(buf->base);
}

static void 
client_write(uv_write_t *req, int status)
{
      if (status)
            fprintf(stdout, "[ERROR] Write error: %s\n", uv_strerror(status));
      free_write_req(req);
}


static void 
on_connect(uv_connect_t* req, int status)
{
      if(status < 0)
      {
            fprintf(stderr, "[ERROR] Connection error: %s\n", uv_strerror(status));
            return;
      }
      fprintf(stdout, "[INFO] Connect to remote server\n");
      uv_read_start((uv_stream_t *)&client, alloc_buffer, read_cb);
}

static void *
utun_read_process(void *args)
{
      for(;;)
      {
            char buffer[2 * BUFSIZ];
            memset(buffer, 0, 2 * BUFSIZ);
            int ret;
            uv_stream_t * stream = (uv_stream_t *)&client;
            if ((ret = utun_read(_conf.utun_fd, buffer)) > 0){
                  write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
                  req->buf = uv_buf_init(buffer, ret);
                  if((ret = uv_write((uv_write_t *)req, (uv_stream_t *)&client, &req->buf, 1, client_write)) < 0)
                  {
                        printf("[ERROR] %s\n", uv_strerror(ret));
                  } 
            }
      }
}
int client_loop(struct config conf)
{
      _conf = conf;
      pthread_t tun_thread;
      
      if (pthread_create(&tun_thread, NULL, utun_read_process, NULL) < 0)
      {
            fprintf(stderr, "[ERROR] Create the thread failed\n");
            return (ERROR);
      }

      loop = uv_default_loop();
      uv_tcp_init(loop, &client);
      struct sockaddr_in addr;
      uv_connect_t *connect = (uv_connect_t *)malloc(sizeof(uv_connect_t));

      uv_ip4_addr(_conf.server_host, _conf.server_port, &addr);

      int ret = uv_tcp_connect(connect, &client, (const struct sockaddr *)&addr, on_connect);

      uv_stream_t * stream = (uv_stream_t *)&client; 
      if(ret)
      {
            fprintf(stderr, "[ERROR] Connect error: %s\n", uv_strerror(ret));
            return 1;
      }
      return uv_run(loop, UV_RUN_DEFAULT);
      
      
}