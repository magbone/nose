
#include "nose.h"
#include "net_config.h"


/*
* Set the ip address of both side of Point-to-point given interface.
*/

int
set_ip_configure(char * interface, char *src_addr, char *dst_addr)
{

      char command_line[100];
      memset(command_line, 0, 100);
      sprintf(command_line, "ifconfig %s %s %s up", interface, src_addr, dst_addr);
      
      if (system(command_line) < 0){
            perror("[ERROR] Execute ifconfig failed");
            return (ERROR);
      }
      return (OK);
}