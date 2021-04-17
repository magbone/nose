#ifndef _UDP_H_
#define _UDP_H_

#include <uv.h>

struct udp_handler
{
      struct sockaddr_in bind_in;
      int bind_flag;
      /** **/
      int sockfd;
      struct sockaddr_in in;
};

int send_udp_pkt(struct udp_handler *handler, char *dst_host, int port, int timeout, char *buf, int size);

int recv_udp_pkt(struct udp_handler *handler, char *buf);

#endif // !_UDP_H_