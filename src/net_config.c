
#include "nose.h"
#include "net_config.h"
#include <unistd.h>

/*
* Set the ip address of both side of Point-to-point given interface.
*/

int
set_ip_configure(char * interface, char *src_addr, char *dst_addr)
{
      char command_line[100];
      
#if defined(__linux)
      int try = 0;
      char _real[10];
      do{
            memset(command_line, 0, 100);
            memset(_real, 0, 10);
            sprintf(_real, "tun%d", try);
            sprintf(command_line, "ip tuntap add mode tun dev %s", _real);
            if (system(command_line) >= 0)
            {
                  strncpy(interface, _real, strlen(_real));
                  break;
            }
      } while (try < 255);
      if (try >= 255)
      {
            fprintf(stderr,  "[ERROR] Add a new tun device failed\n");
            return (FAILED);
      }
#endif
      memset(command_line, 0, 100);
#if defined(__linux)
      sprintf(command_line, "ifconfig %s %s pointopoint %s up", interface, src_addr, dst_addr);
#elif defined(__APPLE__)
      sprintf(command_line, "ifconfig %s %s %s up", interface, src_addr, dst_addr);
#endif      
      if (system(command_line) < 0){
            perror("[ERROR] Execute ifconfig failed");
            return (ERROR);
      }
      return (OK);
}
