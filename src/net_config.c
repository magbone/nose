
#include "nose.h"
#include "net_config.h"
#include <unistd.h>
#include <errno.h>

/*
* Set the ip address of both side of Point-to-point given interface.
*/

int
set_ip_configure(char * interface, char *src_addr, char *dst_addr)
{
      char command_line[100];
      
// #if defined(__linux)
//       int try = 0;
//       char _real[10], result[100];
//       FILE *pfp = NULL;
//       do{
//             memset(command_line, 0, 100);
//             memset(result, 0, 100);
//             memset(_real, 0, 10);
//             sprintf(_real, "tun%d", try++);
//             sprintf(command_line, "ip tuntap add mode tun dev %s", _real);
//             printf("1\n");
//             if ((pfp = popen(command_line, "r")) == NULL)
//             {
//                   fprintf(stderr, "[ERROR] (popen) err_code: %d, err_msg: %s\n", errno, strerror(errno));
//                   return (ERROR);
//             }
//             printf("2\n");
//             while(fgets(result, 100, pfp) != NULL);
//             printf("%d\n", strlen(result));
//             if (!strlen(result))
//             {
//                   strncpy(interface, _real, strlen(_real));
//                   break;
//             }
//             else 
//                   fprintf(stderr, "[ERROR] (popen) %s", result);

//             pclose(pfp);
//       } while (try < 255);
//       if (try >= 255)
//       {
//             fprintf(stderr,  "[ERROR] Add a new tun device failed\n");
//             return (FAILED);
//       }
// #endif
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
