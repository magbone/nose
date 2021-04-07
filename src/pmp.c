
#include <string.h>
#include <arpa/inet.h>

#include "pmp.h"


int 
PMP_discovery_req_pkt(u_int16_t port, char *source_id, char *target_id, char *buf)
{
      PMP_discovery_req_t *dreq = (PMP_discovery_req_t *)malloc(sizeof(PMP_discovery_req_t));

      if (dreq == NULL || buf == NULL) return (ERROR);

      dreq->header.type = DISC;
      dreq->header.code = REQ;
      dreq->port = htons(port);

      memcpy(dreq->source_id, source_id, strlen(source_id));
      memcpy(dreq->target_id, target_id, strlen(target_id));

      memcpy(buf, (char *)dreq, sizeof(PMP_discovery_req_t));
      free(dreq);
      return (sizeof(PMP_discovery_req_t));
}


int 
PMP_discovery_rsp_pkt(char *target_id, struct bucket_item *items, int size, char *buf)
{
      PMP_discovery_rsp_t *drsp = (PMP_discovery_rsp_t *)malloc(sizeof(PMP_discovery_rsp_t));
      PMP_options_t *opt = (PMP_options_t *)malloc(sizeof(PMP_options_t));
      int len = 0;

      if (drsp == NULL || buf == NULL || opt == NULL) return (ERROR);

      drsp->header.type = DISC;
      drsp->header.code = RSP;
      drsp->count = htons(size);
      memcpy(drsp->target_id, target_id, strlen(target_id));

      memcpy(buf, (char *)drsp, sizeof(PMP_discovery_rsp_t));
      
      len += sizeof(PMP_discovery_rsp_t);

      for (int i = 0; i < size; i++)
      {
            opt->port = htons((items + i)->port);
            opt->ipv4 = inet_addr((items + i)->ipv4);
            printf("%04x\n", opt->ipv4);
            memcpy(opt->node_id, (items + i)->node_id, 20);
            
            memcpy(buf + len, (char *)opt, sizeof(PMP_options_t));
            len += sizeof(PMP_options_t);
      }
      
      free(drsp);
      free(opt);
      return (len);
}

int 
PMP_ping_req_pkt(char *source_id, char *target_id, char *buf)
{
      PMP_ping_req_t *preq = (PMP_ping_req_t *)malloc(sizeof(PMP_ping_req_t));

      if (source_id == NULL || target_id == NULL || preq == NULL) return(ERROR);

      preq->header.type = PING;
      preq->header.code = REQ;
      preq->reserve = 0;
      memcpy(preq->source_id, source_id, 20);
      memcpy(preq->target_id, target_id, 20);

      memcpy(buf, (char *)preq, sizeof(PMP_ping_req_t));

      free(preq);
      return (sizeof(PMP_ping_req_t));
}

int 
PMP_ping_rsp_pkt(char *source_id, char *target_id, char *buf)
{
      PMP_ping_rsp_t *prsp = (PMP_ping_rsp_t *)malloc(sizeof(PMP_ping_rsp_t));

      if (source_id == NULL || target_id == NULL || prsp == NULL) return(ERROR);

      prsp->header.type = PING;
      prsp->header.code = RSP;
      memcpy(prsp->source_id, source_id, 20);
      memcpy(prsp->target_id, target_id, 20);

      memcpy(buf, (char *)prsp, sizeof(PMP_ping_req_t));

      free(prsp);
      return (sizeof(PMP_ping_req_t));
}


int 
PMP_discovery_req_unpack(u_int16_t *port, char *source_id, char *target_id, char *buf, int size)
{
      if (size < sizeof(PMP_ping_req_t)) return (ERROR);

      PMP_discovery_req_t *dreq = (PMP_discovery_req_t *)buf;

      if (strcmp((char *)dreq->target_id, source_id)) return (ERROR);
      strncpy(target_id, (char *)dreq->source_id, 20);

      *port = dreq->port;
      return (OK);
}

int 
PMP_discovery_rsp_unpack(char *source_id, struct bucket *b, char *buf, int size)
{
      if (size < sizeof(PMP_discovery_req_t)) return (ERROR);
      PMP_discovery_rsp_t *drsp = (PMP_discovery_rsp_t *)buf;

      if (strncpy(source_id, (char *)drsp->target_id, 20)) return (ERROR);

      int count = drsp->count;
      int offset = sizeof(PMP_discovery_rsp_t);
      struct bucket_item item; 

      if (size < count * sizeof(PMP_options_t) + sizeof(PMP_discovery_rsp_t)) return (ERROR);

      for (int i = 0; i < count; i++)
      {
            PMP_options_t *opt = (PMP_options_t *)(buf + offset);
            item.port = opt->port;
            memcpy(item.node_id, opt->node_id, 20);
            struct in_addr in;
            in.s_addr = opt->ipv4;
            char *ipv4 = inet_ntoa(in);
            memcpy(item.ipv4, ipv4, strlen(ipv4));
            push_front_bucket(b, item);
      }
      
      return (OK);
}


int 
PMP_ping_req_unpack(char *source_id, char *target_id, char *buf, int size)
{
      if (size < sizeof(PMP_ping_req_t)) return (ERROR);

      PMP_ping_req_t *preq = (PMP_ping_req_t *)buf;

      memcpy(source_id, preq->source_id, 20);
      memcpy(target_id, preq->target_id, 20);

      return (OK);
}

