
#include <string.h>
#include <arpa/inet.h>

#include "pmp.h"


int 
PMP_discovery_req_pkt(u_int16_t port, char *source_id, char *target_id, char *buf)
{
      PMP_discovery_req_t *dreq = (PMP_discovery_req_t *)
                  malloc(sizeof(PMP_discovery_req_t));

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
      PMP_discovery_rsp_t *drsp = (PMP_discovery_rsp_t *)
                  malloc(sizeof(PMP_discovery_rsp_t));
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
PMP_get_peers_req_pkt(char *source_id, char *target_id, char *buf)
{
      if (buf == NULL || source_id == NULL || target_id == NULL)
            return (ERROR);

      PMP_get_peers_req_t* gp_req = (PMP_get_peers_req_t *)
                  malloc(sizeof(PMP_get_peers_req_t));

      if (gp_req == NULL) return (ERROR);  

      gp_req->header.type = G_PS;
      gp_req->header.code = REQ;
      gp_req->reserve = 0;
      memcpy(gp_req->source_id, source_id, strlen(source_id));
      memcpy(gp_req->target_id, target_id, strlen(target_id));
      
      memcpy(buf, (char *)gp_req, sizeof(PMP_get_peers_req_t));
      free(gp_req);
      return (sizeof(PMP_get_peers_req_t));
}

int 
PMP_get_peers_rsp_pkt(char *target_id, struct bucket_item *items, int size, char *buf)
{
      int buf_size = 0;

      if (target_id == NULL || items == NULL || buf == NULL || size <= 0) return (ERROR);

      PMP_get_peers_rsp_t* gp_rsp = (PMP_get_peers_rsp_t *)
                  malloc(sizeof(PMP_get_peers_rsp_t));
      if (gp_rsp == NULL) return (ERROR);

      gp_rsp->header.type = G_PS;
      gp_rsp->header.code = RSP;

      gp_rsp->count = htons(size);
      memcpy(gp_rsp->target_id, target_id, strlen(target_id));
      memcpy(buf, (char *)gp_rsp, sizeof(PMP_get_peers_rsp_t));
      buf_size += sizeof(PMP_get_peers_rsp_t);

      PMP_get_peers_options_t* opt = (PMP_get_peers_options_t *)
                  malloc(sizeof(PMP_get_peers_options_t));

      if (opt == NULL)
      {
            if (gp_rsp != NULL) free(gp_rsp);
            return (ERROR);
      }
      
      for (int i = 0; i < size; i++)
      {
            memcpy(opt->node_id, (items + i)->node_id, 20);
            opt->ipv4 = inet_addr((items + i)->ipv4);
            opt->vlan_ipv4 = inet_addr((items + i)->vlan_ipv4);
            opt->nat_type = (items + i)->nat_type;
            opt->port = htons((items + i)->port);

            memcpy(buf + buf_size, (char *)opt, sizeof(PMP_get_peers_options_t));
            buf_size += sizeof(PMP_get_peers_options_t);
      }
      
      free(gp_rsp);
      free(opt);
      return (buf_size);
}

int 
PMP_peer_registry_req_pkt(char *mstp_id, char *peer_id, char *vlan_ipv4, struct nat_type type, char *buf)
{
      if (mstp_id == NULL || peer_id == NULL || vlan_ipv4 == NULL || buf == NULL) return (ERROR);

      PMP_peer_registry_req_t *pr = (PMP_peer_registry_req_t *)malloc(sizeof(PMP_peer_registry_req_t));

      if (pr == NULL) return (ERROR);

      pr->header.type = R_P;
      pr->header.code = REQ;

      pr->nat_type = htons(type.nat_type);
      pr->port = htons(type.port);
      pr->ipv4 = inet_addr(type.ipv4);
      pr->vlan_ipv4 = inet_addr(vlan_ipv4);

      memcpy(pr->mstp_id, mstp_id, 20);
      memcpy(pr->peer_id, peer_id, 20);

      memcpy(buf, (char *)pr, sizeof(PMP_peer_registry_req_t));
      free(pr);

      return sizeof(PMP_peer_registry_req_t);
}

int 
PMP_peer_registry_rsp_pkt(char *mstp_id, char *peer_id, char *buf)
{
      if (mstp_id == NULL || peer_id == NULL || buf == NULL) return (ERROR);

      PMP_peer_registry_rsp_t *pr = (PMP_peer_registry_rsp_t *)
                  malloc(sizeof(PMP_peer_registry_rsp_t));

      if (pr == NULL) return (ERROR);

      pr->header.type = R_P;
      pr->header.code = RSP;

      memcpy(pr->source_id, mstp_id, 20);
      memcpy(pr->target_id, peer_id, 20);

      memcpy(buf, (char *)pr, sizeof(PMP_peer_registry_rsp_t));

      free(pr);
      return (sizeof(PMP_peer_registry_rsp_t));


}

int 
PMP_find_peer_req_pkt(char *peer_id, char *mstp_id, char *vlan_ipv4, char *buf)
{
      if (peer_id == NULL || mstp_id == NULL || vlan_ipv4 == NULL || buf == NULL) return (ERROR);

      PMP_find_peer_req_t *fpreq = 
                  (PMP_find_peer_req_t *)malloc(sizeof (PMP_find_peer_req_t));
      if (fpreq == NULL)
            return (ERROR);
      
      fpreq->header.type = F_P;
      fpreq->header.code = REQ;

      fpreq->vlan_ipv4   = inet_addr(vlan_ipv4);
      memcpy(fpreq->mstp_id, mstp_id, 20);
      memcpy(fpreq->peer_id, peer_id, 20);

      memcpy(buf, (char *)fpreq, sizeof(PMP_find_peer_req_t));
      free(fpreq);

      return (sizeof(PMP_find_peer_req_t));
}

int 
PMP_find_peer_rsp_pkt(char *mstp_id, char *peer_id, struct bucket_item item, char *buf)
{
      if (mstp_id == NULL || peer_id == NULL || buf == NULL) return (ERROR);

      PMP_find_peer_rsp_t *fprsp = 
                  (PMP_find_peer_rsp_t *)malloc(sizeof(PMP_find_peer_rsp_t));
            
      if (fprsp == NULL) return (ERROR);

      fprsp->header.type = F_P;
      fprsp->header.code = RSP;
      
      fprsp->ipv4 = inet_addr(item.ipv4);
      fprsp->port = htons(item.port);
      fprsp->nat_type = htons(item.nat_type);
      fprsp->vlan_ipv4 = inet_addr(item.vlan_ipv4);
      memcpy(fprsp->mstp_id, mstp_id, 20);
      memcpy(fprsp->peer_id, peer_id, 20);

      memcpy(buf, (char *)fprsp, sizeof(PMP_find_peer_rsp_t));
      free(fprsp);

      return (sizeof(PMP_find_peer_rsp_t));
}

int 
PMP_discovery_req_unpack(u_int16_t *port, char *source_id, char *target_id, char *buf, int size)
{
      if (size < sizeof(PMP_discovery_req_t)) return (ERROR);

      PMP_discovery_req_t *dreq = (PMP_discovery_req_t *)buf;

      //if (strcmp((char *)dreq->target_id, source_id)) return (ERROR);
      strncpy(target_id, (char *)dreq->target_id, 20);
      strncpy(source_id, (char *)dreq->source_id, 20);

      target_id[20] = '\0';
      source_id[20] = '\0';
      *port = dreq->port;
      return (OK);
}

int 
PMP_discovery_rsp_unpack(char *source_id, struct bucket *b, char *buf, int size)
{
      if (size < sizeof(PMP_discovery_req_t)) return (ERROR);
      PMP_discovery_rsp_t *drsp = (PMP_discovery_rsp_t *)buf;

      // if (strncpy(source_id, (char *)drsp->target_id, 20)) return (ERROR);

      const int count = drsp->count;
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
            item.ipv4[strlen(ipv4)] = '\0';
            push_front_bucket(b, item);
            offset += sizeof(PMP_options_t);
      }
      
      return (OK);
}


int 
PMP_ping_req_unpack(char *source_id, char *target_id, char *buf, int size)
{
      if (size < sizeof(PMP_ping_req_t)) return (ERROR);

      PMP_ping_req_t *preq = (PMP_ping_req_t *)buf;

      strncpy(source_id, (char *)preq->source_id, 20);
      strncpy(target_id, (char *)preq->target_id, 20);

      source_id[20] = '\0';
      target_id[20] = '\0';
      return (OK);
}


int 
PMP_get_peers_rsp_unpack(char *source_id, struct bucket *b, char *buf, int size)
{
      if (size < sizeof(PMP_get_peers_rsp_t)) return (ERROR);

      PMP_get_peers_rsp_t* gp_rsp = (PMP_get_peers_rsp_t *)buf;
      const int count = htons(gp_rsp->count);
      struct bucket_item item;

      if (size < sizeof(PMP_get_peers_rsp_t) + sizeof(PMP_get_peers_options_t) * count) return (ERROR);

      buf += sizeof(PMP_get_peers_rsp_t);

      for (int i = 0; i < count; i++)
      {
            PMP_get_peers_options_t *gp_opt = (PMP_get_peers_options_t *)buf;
            item.port = htons(gp_opt->port);
            item.nat_type = gp_opt->nat_type;
            memcpy(item.node_id, gp_opt->node_id, 20);
            struct in_addr in;
            in.s_addr = gp_opt->ipv4;
            char *ipv4 = inet_ntoa(in);
            memcpy(item.ipv4, ipv4, strlen(ipv4));
            item.ipv4[strlen(ipv4)] = '\0';

            in.s_addr = gp_opt->vlan_ipv4;
            ipv4 = inet_ntoa(in);
            memcpy(item.vlan_ipv4, ipv4, strlen(ipv4));
            item.vlan_ipv4[strlen(ipv4)] = '\0';
            #ifdef DEBUG
            fprintf(stdout, "[DEBUG] Got peer %s:%d|%s vlan_ipv4:%s\n", 
                        item.ipv4, item.port, item.node_id, item.vlan_ipv4);
            #endif // DEBUG
            push_front_bucket(b, item);
            buf += sizeof(PMP_get_peers_options_t);
      }
      
      return (OK);
}

int 
PMP_peer_registry_req_unpack(char *peer_id, struct bucket *b, char *buf, int size)
{
      if (peer_id == NULL || b == NULL || buf == NULL) return (ERROR);

      if (size < sizeof(PMP_peer_registry_req_t)) return (ERROR);

      PMP_peer_registry_req_t  *pr = (PMP_peer_registry_req_t *)buf;
      struct bucket_item item;

      item.nat_type = htons(pr->nat_type);
      item.port = htons(pr->port);
      struct in_addr in;
      in.s_addr = pr->ipv4;
      char *ipv4 = inet_ntoa(in);
      memcpy(item.ipv4, ipv4, strlen(ipv4));
      item.ipv4[strlen(ipv4)] = '\0';

      in.s_addr = pr->vlan_ipv4;
      ipv4 = inet_ntoa(in);
      memcpy(item.vlan_ipv4, ipv4, strlen(ipv4));
      item.vlan_ipv4[strlen(ipv4)] = '\0';
      memcpy(item.node_id, pr->peer_id, 20);
      item.node_id[20] = '\0';
      memcpy(peer_id, item.node_id, 20);
      peer_id[20] = '\0';
      push_front_bucket(b, item);
      return (OK);
}

int 
PMP_find_peer_req_unpack(char *peer_id, char *mstp_id, struct bucket *b, struct bucket_item *item, char *buf, int size)
{
      if (peer_id == NULL || mstp_id == NULL || item == NULL || b == NULL || buf == NULL) return (ERROR);

      if (size < sizeof(PMP_find_peer_req_t)) return (ERROR);

      PMP_find_peer_req_t *fpreq = (PMP_find_peer_req_t *)buf;

      struct in_addr in;
      in.s_addr = fpreq->vlan_ipv4;
      char ipv4[16] = {0};
      strcpy(ipv4, inet_ntoa(in));

      memcpy(peer_id, fpreq->peer_id, 20);
      memcpy(mstp_id, fpreq->mstp_id, 20);

      peer_id[20] = mstp_id[20] = '\0';
      return (get_item_by_vlan_ipv4(b, ipv4, item));
}

