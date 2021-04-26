
#include "pcp.h"
#include "nose.h"

int 
PCP_hello_syn(char *buf)
{
      struct PCP *pcp = (struct PCP *)malloc(sizeof(struct PCP));

      if (pcp == NULL) return (ERROR);

      pcp->ver = 0;
      pcp->flags = F_HELLO_SYN;
      pcp->len = 0;

      memcpy(buf, (char *)pcp, sizeof(struct PCP));
      free(pcp);
      return (sizeof(struct PCP));
}

int PCP_hello_ack(char *buf)
{
      struct PCP *pcp = (struct PCP *)malloc(sizeof(struct PCP));

      if (pcp == NULL) return (ERROR);

      pcp->ver = 0;
      pcp->flags = F_HELLO_ACK;
      pcp->len = 0;

      memcpy(buf, (char *)pcp, sizeof(struct PCP));
      free(pcp);
      return (sizeof(struct PCP));
}

int 
PCP_hb_syn(char *buf)
{
      struct PCP *pcp = (struct PCP *)malloc(sizeof(struct PCP));

      if (pcp == NULL) return (ERROR);

      pcp->ver = 0;
      pcp->flags = F_HB_SYN;
      pcp->len = 0;

      memcpy(buf, (char *)pcp, sizeof(struct PCP));
      free(pcp);
      return (sizeof(struct PCP));
}

int 
PCP_hb_ack(char *buf)
{
      struct PCP *pcp = (struct PCP *)malloc(sizeof(struct PCP));

      if (pcp == NULL) return (ERROR);

      pcp->ver = 0;
      pcp->flags = F_HB_ACK;
      pcp->len = 0;

      memcpy(buf, (char *)pcp, sizeof(struct PCP));
      free(pcp);
      return (sizeof(struct PCP));
}


int 
PCP_payload_pkt(char *buf,char *bbuf, int all_size, uint16_t text_size)
{
      if (buf == NULL || bbuf == NULL) return (ERROR);
      struct PCP *pcp = (struct PCP *)malloc(sizeof(struct PCP));
      uint16_t _text_size = htons(text_size); 
      if (pcp == NULL) return (ERROR);

      pcp->ver = 0;
      pcp->flags = F_PAYLOAD;
      pcp->len = htons(all_size + sizeof(uint16_t));

      memcpy(bbuf, (char *)pcp, sizeof(struct PCP));
      memcpy(bbuf + sizeof(struct PCP), (char *)&_text_size, sizeof(_text_size));
      memcpy(bbuf + sizeof(struct PCP) + sizeof(uint16_t), buf, all_size);
      free(pcp);

      return (sizeof(struct PCP) + sizeof(uint16_t) + all_size);
}
