
#ifndef _CONFIG_
#define _CONFIG_
struct config
{
      char *server_host;
      int server_port;
      int utun_fd;
      char *local_host;
      char *remote_host;
};

#endif // !_CONFIG_