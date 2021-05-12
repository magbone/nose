#include "../src/udp.h"
#include <unistd.h>

int main()
{
      char buf[1024];
      for(;;)
      {
            struct udp_handler hd;
            if (send_udp_pkt(&hd, "10.1.0.3", 9998, 1, buf, 1024) <= 0)
            {
                  printf("send failed\n");
                  goto closed;
            }
            sleep(1);
            printf("send ok\n");
            closed: close(hd.sockfd);
      }
      return 0;
}