
#include "udp.h"
#include "nose.h"


int 
send_udp_pkt(struct udp_handler * handler, char *dst_host, int port, int timeout, char *buf, int size)
{
      if (handler == NULL) return (ERROR);
      int sockfd, addr_len = sizeof(struct sockaddr_in);
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr(dst_host);
      addr.sin_port = htons(port);

      if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) return (ERROR);

      struct timeval tv;
      tv.tv_sec = timeout;
      tv.tv_usec = 0;

      if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
            return (ERROR);
      
      if (sendto(sockfd, buf, size, 0, (struct sockaddr *)&addr, addr_len) < 0)
            return (ERROR);

      handler->sockfd = sockfd;
      handler->in = addr;

      return (OK);
}

int recv_udp_pkt(struct udp_handler *handler, char *buf)
{
      if (handler == NULL) return (ERROR);
      int addr_len = sizeof(struct sockaddr);
      int ret = recvfrom(handler->sockfd, buf, BUFSIZ, 0, (struct sockaddr *)&handler->in, (socklen_t *)& addr_len);

      return ret;

}