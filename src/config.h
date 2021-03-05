
#ifndef _CONFIG_H_
#define _CONFIG_H_
struct config
{
      char *server_host;
      int server_port;
      int utun_fd;
      char *local_host; //
      char *remote_host; //
      char *key; // hashed value with end of zero, the total length is 65 bytes.
      int id; // Session id(also call id) initialzed by every beginning connectin.
      int challenge; // A random number for authentication
};

#endif // !_CONFIG_H_