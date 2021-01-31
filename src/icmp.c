
#include "icmp.h"
#include "nose.h"
#include "utils.h"

#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

int 
is_icmp_echo_req(char *buffer)
{
      struct ip *ip_hdr = (struct ip *)buffer;
      
      if (ip_hdr->ip_p != IPPROTO_ICMP)
            return (0);

      struct icmp *icmp = (struct icmp *)(buffer + 4 * ip_hdr->ip_hl);
      if (icmp->icmp_type != ICMP_ECHO || icmp->icmp_code != 0)
            return (0);
      return 1;
}

void
icmp_echo_reply(char *buffer, int len)
{
      char tmp[len];
      memset(tmp, 0, len);

      struct ip* ip_hdr = (struct ip *)buffer;
      swap(&ip_hdr->ip_dst, &ip_hdr->ip_src);
      struct icmp* icmp = (struct icmp *)(buffer + 4 * ip_hdr->ip_hl);
      icmp->icmp_type = ICMP_ECHOREPLY;

      ip_hdr->ip_sum = checksum_cmpt((char *)ip_hdr, 4 * ip_hdr->ip_hl);
      icmp->icmp_cksum = checksum_cmpt((char *)icmp, ip_hdr->ip_len - 4 * ip_hdr->ip_hl);

      memcpy(tmp, (char *)ip_hdr, 4 * ip_hdr->ip_hl);
      memcpy(tmp + 4 * ip_hdr->ip_hl, (char *)icmp, ip_hdr->ip_len - 4 * ip_hdr->ip_hl);

      memcpy(buffer, tmp, len);
}