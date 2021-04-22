
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
PCP_payload_pkt(char *buf,char *bbuf, int size)
{
      if (buf == NULL || bbuf == NULL) return (ERROR);
      struct PCP *pcp = (struct PCP *)malloc(sizeof(struct PCP));

      if (pcp == NULL) return (ERROR);

      pcp->ver = 0;
      pcp->flags = F_PAYLOAD;
      pcp->len = htons(size);

      memcpy(bbuf, (char *)pcp, sizeof(struct PCP));
      memcpy(bbuf + sizeof(struct PCP), buf, size);
      free(pcp);

      return (sizeof(struct PCP) + size);
}
