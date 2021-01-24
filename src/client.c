

#include "client.h"
#include "device.h"
#include <unistd.h>

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
            buf->base[nread] = 0;
            fprintf(stdout, "[INFO] Receive data length: %ld\n", buf->len);

            char buffer[buf->len];
            memcpy(buffer, buf->base, buf->len);
            utun_write(_conf.utun_fd, buffer, buf->len);
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
utun_poll_cb(uv_poll_t *handle, int status, int event)
{
      printf("7\n");
      if(status < 0){
            fprintf(stderr, "[ERROR] Read error: %s\n", uv_strerror(status));
            return;
      }

      char buffer[BUFSIZ * 2];
      int ret;

      switch (event)
      {
      case UV_READABLE:
            
            if ((ret = utun_read(_conf.utun_fd, buffer)) > 0){
                  write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
                  req->buf = uv_buf_init(buffer, ret);
                  uv_write((uv_write_t *)req, (uv_stream_t *)&client, &req->buf, 1, client_write); 
            }
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
            fprintf(stderr, "[ERROR] Connection error: %s\n", uv_strerror(status));
            return;
      }
      fprintf(stdout, "[INFO] Connect to remote server\n");
      uv_read_start((uv_stream_t *)&client, alloc_buffer, read_cb);
}

static int
utun_read_process()
{
      for(;;)
      {
            char buffer[2 * BUFSIZ];
            int ret;
            uv_stream_t * stream = (uv_stream_t *)&client; 
            if ((ret = utun_read(_conf.utun_fd, buffer)) > 0){
                  write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
                  req->buf = uv_buf_init(buffer, ret);
                  uv_write((uv_write_t *)req, (uv_stream_t *)&client, &req->buf, 1, client_write); 
            }
            

      }
      return (OK);
}
int client_loop(struct config conf)
{
      loop = uv_default_loop();
      uv_tcp_init(loop, &client);
      _conf = conf;
      
      pid_t pid = fork();
      if (pid < 0)
            return (FAILED);
      
      else if (pid == 0)
            return utun_read_process();
      else
      {
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
      
}