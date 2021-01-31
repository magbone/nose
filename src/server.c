

#include "server.h"

static void
add_client(uv_tcp_t *client)
{
      for(int i = 0; i < 2; i++){
            if (clients[i] == NULL)
            {
                  clients[i] = client;
                  break;
            }

      }
}

static void 
remove_client(uv_tcp_t *client)
{
      for(int i = 0; i < 2; i++)
      {
            if (clients[i] == client)
            {
                  clients[i] = NULL;
                  break;
            }
      }
}

static uv_tcp_t * 
get_other_client(uv_tcp_t *client)
{
      for(int i = 0; i < 2; i++)
            if (clients[i] != client) return clients[i];
      return NULL;
}

void 
alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
      buf->len = suggested_size;
      buf->base = (char *)malloc(suggested_size);
}

static void 
on_close(uv_handle_t *handle)
{
      if (handle != NULL)
      {
            remove_client((uv_tcp_t *)handle);
            connection_count--;
            free(handle);
      }
}

void 
free_write_req(uv_write_t *req)
{
      write_req_t *wr = (write_req_t *)req;
      // The following code will trigger the error of pointer being freed was not allocated
      // if (wr->buf.base != NULL)
      //       free(wr->buf.base);
      free(wr);
}

static void 
forwarding_write(uv_write_t *req, int status)
{
      if (status)
            fprintf(stdout, "[ERROR] Write error: %s\n", uv_strerror(status));
      free_write_req(req);
}

static void 
forwarding_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
      if(nread > 0)
      {
            buf->base[nread] = 0;
            fprintf(stdout, "[INFO] Receive data length: %ld\n", nread);

            if (connection_count != 2) return;
            
            write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
            req->buf = uv_buf_init(buf->base, nread);
            uv_write((uv_write_t *)req, (uv_stream_t *)get_other_client((uv_tcp_t *)client), &req->buf, 1, forwarding_write);
            return;
            
      }
      else if(nread < 0)
      {
            if(nread != UV_EOF)
                  fprintf(stderr, "[ERROR] Read error: %s\n", uv_err_name(nread));
            else
                  fprintf(stderr, "[ERROR] Client disconnect\n");

            uv_close((uv_handle_t *)client, on_close);
      }

      if (buf->base != NULL) free(buf->base);
}
static void 
on_new_connection(uv_stream_t * server, int status)
{
      if (status < 0)
      {
            fprintf(stderr, "[ERROR] New conncetion error: %s\n", uv_strerror(status));
            return;
      }

      uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
      uv_tcp_init(loop, client);

      if(uv_accept(server, (uv_stream_t *)client) == 0 && connection_count < 2)
      {
            add_client(client);
            connection_count++;

            // Get connected peer's information
            struct sockaddr_in sock_addr;
            int len = sizeof(struct sockaddr_in);
            uv_tcp_getpeername(client, (struct sockaddr *)&sock_addr, &len);
            fprintf(stdout, "[INFO] Connected client: %s:%d\n", inet_ntoa(sock_addr.sin_addr), sock_addr.sin_port);
            
            uv_read_start((uv_stream_t *)client, alloc_buffer, forwarding_read);
      }
      else
      {
            fprintf(stdout, "[INFO] Out of number of connections\n");
            uv_close((uv_handle_t *)client, NULL);
      }


}
int server_loop(struct config conf)
{
      
      loop = uv_default_loop();

      uv_tcp_t server;

      uv_tcp_init(loop, &server);

      
      struct sockaddr_in addr;
      uv_ip4_addr(conf.server_host, conf.server_port, &addr);

      fprintf(stdout, "[INFO] Server listens at %s:%d\n", conf.server_host, conf.server_port);
      uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
      int ret = uv_listen((uv_stream_t *)&server, 128, on_new_connection);

      if (ret)
      {
            fprintf(stderr, "[ERROR] Listen eeror: %s\n", uv_strerror(ret));
            return 1;
      }
      return uv_run(loop, UV_RUN_DEFAULT);
}
