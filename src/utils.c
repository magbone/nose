




#include "utils.h"
#include <arpa/inet.h>
#include <ctype.h>

int 
is_valid_ip_address(char *ip)
{
      struct sockaddr_in sa;
      int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
      return result != 0;
}

