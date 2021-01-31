#ifndef _ICMP_H_
#define _ICMP_H_

int is_icmp_echo_req(char *buffer);

void icmp_echo_reply(char *buffer, int len);

#endif // !_ICMP_H_