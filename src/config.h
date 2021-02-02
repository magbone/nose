
#ifndef _CONFIG_H_
#define _CONFIG_H_
struct config
{
      char *server_host;
      int server_port;
      int utun_fd;
      char *local_host;
      char *remote_host;
      char *key;
      int id;
      int challenge;
};

#endif // !_CONFIG_H_