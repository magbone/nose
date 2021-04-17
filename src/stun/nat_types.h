#ifndef _TYPES_H_
#define _TYPES_H_

#define UDP_BLOCKED           0
#define SYM_UDP_FIREWALL      1
#define OPEN_INTERNET         2
#define FULL_CONE             3
#define RESTRICTED            4
#define PORT_RESTRICTED       5
#define SYM_NAT               6
#define UNEXPECTED_NAT_TYPE   7

struct nat_type
{
      int nat_type;
      int port;
      char ipv4[16];
};




#endif // !_TYPES_H_