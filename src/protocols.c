
#include "protocols.h"
#include "nose.h"

#include <netinet/ip.h>
#include <arpa/inet.h>


struct ip* 
filling_IP_header(char *dst, int protocol, int payload){
      struct ip * ip_header = (struct ip *)malloc(sizeof(struct ip));

      if (ip_header == NULL) return NULL;

      ip_header->ip_v = IPVERSION;
      ip_header->ip_hl = sizeof(struct ip) >> 2;
      ip_header->ip_tos = 0;
      ip_header->ip_len = htons(payload);
      ip_header->ip_id = 0;
      ip_header->ip_off = 0;
      ip_header->ip_ttl = MAXTTL;
      ip_header->ip_p = protocol;
      ip_header->ip_dst.s_addr = inet_addr(dst);
      
      return ip_header;
}

struct EGRE_header* 
filling_EGRE_header(int key_payload_len, int call_id)
{
      struct EGRE_header *egre_header = (struct EGRE_header *)malloc(sizeof(struct EGRE_header));

      if (egre_header ==  NULL) return (NULL);

      egre_header->checksum = 0;
      egre_header->routing = 0;
      egre_header->key = 1;
      egre_header->seq_number = 0;
      egre_header->s_source_route = 0;
      egre_header->recur = 0;
      egre_header->ack = 0;
      egre_header->flags = 0;
      egre_header->ver = 1;
      egre_header->protocol_type = htons(0x880B);
      egre_header->key_payload_len = htons(key_payload_len);

      egre_header->key_call_id = htons(call_id);

      return egre_header;
}


struct PPP * 
filling_PPP(int protocol)
{
      struct PPP *ppp = (struct PPP *)malloc(sizeof(struct PPP));

      if (ppp == NULL) return (NULL);  
      ppp->address = 0xff;
      ppp->control = 0x03;
      ppp->protocol = htons(protocol);

      return ppp;
}

struct PPP_LCP *
filling_PPP_LCP(int code, int id, int len)
{
      struct PPP_LCP *ppp_lcp = (struct PPP_LCP *)malloc(sizeof(struct PPP_LCP));

      if (ppp_lcp == NULL) return (NULL);

      ppp_lcp->code = code;
      ppp_lcp->indentfier = id;
      ppp_lcp->length = htons(len);
      return ppp_lcp;
}