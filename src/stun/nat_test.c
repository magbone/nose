
#include "nat_test.h"
#include "../utils.h"
#include "../nose.h"


#include <errno.h>
#include <arpa/inet.h>

static int 
_create_nat_test_sock(struct sockaddr_in *addr)
{
      int sockfd, ret, addr_len = sizeof(struct sockaddr_in);
      
      if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) return (sockfd);
      struct timeval tv;
      tv.tv_sec = 1;
      tv.tv_usec = 0;

      if ((ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv))) < 0)
            return (ret);
      
      if ((ret = bind(sockfd, (struct sockaddr*)addr, addr_len)) < 0)
            return (ret);
            
      return (sockfd);
}
/**
 * 
                        +--------+
                        |  Test  |
                        |   I    |
                        +--------+
                             |
                             |
                             V
                            /\              /\
                         N /  \ Y          /  \ Y             +--------+
          UDP     <-------/Resp\--------->/ IP \------------->|  Test  |
          Blocked         \ ?  /          \Same/              |   II   |
                           \  /            \? /               +--------+
                            \/              \/                    |
                                             | N                  |
                                             |                    V
                                             V                    /\
                                         +--------+  Sym.      N /  \
                                         |  Test  |  UDP    <---/Resp\
                                         |   II   |  Firewall   \ ?  /
                                         +--------+              \  /
                                             |                    \/
                                             V                     |Y
                  /\                         /\                    |
   Symmetric  N  /  \       +--------+   N  /  \                   V
      NAT  <--- / IP \<-----|  Test  |<--- /Resp\               Open
                \Same/      |   I    |     \ ?  /               Internet
                 \? /       +--------+      \  /
                  \/                         \/
                  |                           |Y
                  |                           |
                  |                           V
                  |                           Full
                  |                           Cone
                  V              /\
              +--------+        /  \ Y
              |  Test  |------>/Resp\---->Restricted
              |   III  |       \ ?  /
              +--------+        \  /
                                 \/
                                  |N
                                  |       Port
                                  +------>Restricted
 * 
*/
static int 
stun_rsp_unpack( char *buf, int size, struct rsp_values *rvals )
{
      if (size < sizeof(struct _STUN_message_header))
            return (ERROR);

      struct _STUN_message_header* header = (struct _STUN_message_header *)buf;
      
      if (size < sizeof(struct _STUN_message_header) + htons(header->message_length))
            return (ERROR);

      int index = sizeof(struct _STUN_message_header);
      struct _STUN_attribute_value *value = NULL;
      struct in_addr in;
      char *ipv4 = NULL, *server_name;


      while (index < sizeof(struct _STUN_message_header) + htons(header->message_length))
      {
            struct _STUN_attribute* attr = (struct _STUN_attribute *)(buf + index);

            index += sizeof(struct _STUN_attribute);

            switch (htons(attr->type))
            {
                  case MAPPED_ADDR:
                        value = (struct _STUN_attribute_value *)(buf + index);
                        in.s_addr = value->ipv4;
                        ipv4 = inet_ntoa(in);
                        #ifdef DEBUG
                        fprintf(stdout, "Mapped Address: %s:%d\n", ipv4, htons(value->port));
                        #endif
                        strncpy( rvals->external_ip, ipv4, strlen( ipv4 ) );
                        *( rvals->external_ip + strlen( ipv4 ) ) = '\0';
                        rvals->external_port = htons( value->port );
                        break;
                  case SRC_ADDR:
                        value = (struct _STUN_attribute_value *)(buf + index);
                        in.s_addr = value->ipv4;
                        ipv4 = inet_ntoa(in);
                        #ifdef DEBUG 
                        fprintf(stdout, "Source Address: %s:%d\n", ipv4, htons(value->port));
                        #endif
                        strncpy( rvals->source_ip, ipv4, strlen( ipv4 ) );
                        *( rvals->source_ip + strlen( ipv4 ) ) = '\0';
                        rvals->source_port = htons( value->port );
                        break;
                  case CHANGE_ADDR:
                        value = (struct _STUN_attribute_value *)(buf + index);
                        in.s_addr = value->ipv4;
                        ipv4 = inet_ntoa(in);
                        #ifdef DEBUG
                        fprintf(stdout, "Changed Address: %s:%d\n", ipv4, htons(value->port));
                        #endif 
                        strncpy( rvals->changed_ip, ipv4, strlen( ipv4 ) );
                        *( rvals->changed_ip + strlen( ipv4 ) ) = '\0';
                        rvals->changed_port = htons( value->port );
                        break;
                  case XOR_MAPPED_ADDR:
                        value = (struct _STUN_attribute_value *)(buf + index);
                        in.s_addr = value->ipv4;
                        ipv4 = inet_ntoa(in);
                        #ifdef DEBUG
                        fprintf(stdout, "Xor Mapped Address: %s:%d\n", ipv4, htons(value->port));
                        #endif
                        break;
                  case SERVER_NAME:
                        server_name = (char *)malloc(sizeof(char) * htons(attr->length));
                        strncpy(server_name, buf + index, htons(attr->length));
                        #ifdef DEBUG
                        fprintf(stdout, "Server Name: %s\n", server_name);
                        #endif
                        free(server_name);
                        break;
                  default:
                        break;
            }
            index += htons(attr->length);
      }
      return (OK);
}

static int test1( int sockfd, struct sockaddr_in *dst_addr,
      struct sockaddr_in *src_addr, char *trans_id,
      struct rsp_values *rvals )
{
      #ifdef DEBUG
      printf("Do testI\n");
      #endif
      struct _STUN_message_header* header = 
                  (struct _STUN_message_header *)malloc(sizeof(struct _STUN_message_header));
      if (header == NULL) return (ERROR);

      header->stun_message_type = htons(BIND_REQ);
      header->message_length = 0;
      memcpy(header->transaction_id, trans_id, 12);

      int addr_len = sizeof(struct sockaddr_in), len;
      char buf[BUFSIZ];
      memcpy(buf, (char *)header, sizeof(struct _STUN_message_header));
      free(header);

      if (sendto(sockfd, buf, sizeof(struct _STUN_message_header), 0, (struct sockaddr *)dst_addr, addr_len) < 0)
      {
            #ifdef DEBUG 
            fprintf(stderr, "[ERROR] testI(sendto) err_code: %d, err_msg: %s\n", errno, strerror(errno));
            #endif 
            return (ERROR);
      }
      
      if ((len = recvfrom(sockfd, buf, BUFSIZ, 0, (struct sockaddr *)src_addr, (socklen_t *)& addr_len)) < 0)
      {
            #ifdef DEBUG 
            fprintf(stderr, "[ERROR] testI(recvfrom) err_code: %d, err_msg: %s\n", errno, strerror(errno));
            #endif 
            return (len);
      }
      
      return stun_rsp_unpack( buf, len, rvals );
}

static int test2( int sockfd, struct sockaddr_in *dst_addr,
      struct sockaddr_in *src_addr, char *trans_id,
      struct rsp_values *rvals )
{
      #ifdef DEBUG
      printf("Do testII\n");
      #endif
      struct _STUN_message_header* header = 
                  (struct _STUN_message_header *)malloc(sizeof(struct _STUN_message_header));
      if (header == NULL) return (ERROR);

      header->stun_message_type = htons(BIND_REQ);
      memcpy(header->transaction_id, trans_id, 12);

      struct _STUN_attribute *attr = 
                  (struct _STUN_attribute *)malloc(sizeof(struct _STUN_attribute));
      if (attr == NULL)
      {
            if (header != NULL) free(header);
            return (ERROR);
      }

      attr->type = htons(CHANGE_REQ);
      attr->length = htons(4);
      u_int32_t flags = htonl(FLAG_CHANGE_IP | FLAG_CHANGE_PORT);

      header->message_length = htons(sizeof(struct _STUN_attribute) + 4);

      char buf[BUFSIZ];
      int addr_len = sizeof(struct sockaddr_in), len;

      memcpy(buf, (char *)header, sizeof(struct _STUN_message_header));
      memcpy(buf + sizeof(struct _STUN_message_header), (char *)attr, 
                  sizeof(struct _STUN_attribute));
      memcpy(buf + sizeof(struct _STUN_message_header) + sizeof(struct _STUN_attribute), 
                   (char *)&flags, sizeof(flags));

      free(header);
      free(attr);

      const int size = sizeof(struct _STUN_message_header) + 
                  sizeof(struct _STUN_attribute) + sizeof(flags);
      
      if ((len = sendto(sockfd, buf, size, 0, (struct sockaddr *)dst_addr, addr_len)) < 0)
      {
            #ifdef DEBUG 
            fprintf(stderr, "[ERROR] testII(sendto) err_code: %d, err_msg: %s\n", errno, strerror(errno));
            #endif 
            return (len);
      }
      
      if ((len = recvfrom(sockfd, buf, BUFSIZ, 0, (struct sockaddr *)src_addr, (socklen_t *)& addr_len)) < 0)
      {
            #ifdef DEBUG 
            fprintf(stderr, "[ERROR] testII(recvfrom) err_code: %d, err_msg: %s\n", errno, strerror(errno));
            #endif 
            return (len);
      }
      
      return stun_rsp_unpack( buf, len, rvals );
}

static int test3( int sockfd, struct sockaddr_in *dst_addr,
      struct sockaddr_in *src_addr, char *trans_id,
      struct rsp_values *rvals )
{
      struct _STUN_message_header* header = 
                  (struct _STUN_message_header *)malloc(sizeof(struct _STUN_message_header));
      if (header == NULL) return (ERROR);

      header->stun_message_type = htons(BIND_REQ);
      memcpy(header->transaction_id, trans_id, 12);

      struct _STUN_attribute *attr = 
            (struct _STUN_attribute *)malloc(sizeof(struct _STUN_attribute));
      if (attr == NULL)
      {
            if (header != NULL) free(header);
            return (ERROR);
      }

      attr->type = htons(CHANGE_REQ);
      attr->length = htons(4);
      u_int32_t flags = htonl(FLAG_CHANGE_PORT);

      header->message_length = htons(sizeof(struct _STUN_attribute) + 4);

      char buf[BUFSIZ];
      memcpy(buf, (char *)header, sizeof(struct _STUN_message_header));
      memcpy(buf + sizeof(struct _STUN_message_header), (char *)attr, 
                  sizeof(struct _STUN_attribute));
      memcpy(buf + sizeof(struct _STUN_message_header) + sizeof(struct _STUN_attribute), 
             (    char *)&flags, sizeof(flags));

      free(header);
      free(attr);

      const int size = sizeof(struct _STUN_message_header) + 
                  sizeof(struct _STUN_attribute) + sizeof(flags);
      int addr_len = sizeof(struct sockaddr_in), len;
      
      if ((len = sendto(sockfd, buf, size, 0, (struct sockaddr *)dst_addr, addr_len)) < 0)
      {
            #ifdef DEBUG 
            fprintf(stderr, "[ERROR] testIII(sendto) err_code: %d, err_msg: %s\n", errno, strerror(errno));
            #endif 
            return (len);
      }
      
      if ((len = recvfrom(sockfd, buf, BUFSIZ, 0, (struct sockaddr *)src_addr, (socklen_t *)& addr_len)) < 0)
      {
            #ifdef DEBUG 
            fprintf(stderr, "[ERROR] testIII(recvfrom) err_code: %d, err_msg: %s\n", errno, strerror(errno));
            #endif 
            return (len);
      }

      return stun_rsp_unpack( buf, len, rvals );
}
static int do_test(const char *source_addr, const int source_port, 
      const char *stun_server_addr, const int stun_server_port,
      struct nat_type *type)
{
      char trans_id[16];
      gen_random_trans_id(trans_id);
      struct sockaddr_in dst_addr, src_addr;
      dst_addr.sin_family = AF_INET;
      dst_addr.sin_addr.s_addr = inet_addr(stun_server_addr);
      dst_addr.sin_port = htons(stun_server_port);

      src_addr.sin_family = AF_INET;
      src_addr.sin_addr.s_addr = inet_addr(source_addr);
      src_addr.sin_port = htons(source_port);


      int sockfd = _create_nat_test_sock(&src_addr);

      if (sockfd < 0) 
      {
            #ifdef DEBUG
            fprintf(stderr, "[ERROR] Create socket failed. err_code:%d, err_msg:%s\n", errno, strerror(errno));
            #endif
            return (sockfd); 
      }

      struct rsp_values rvals = {0};
      // Do TestI
      if ( test1( sockfd, &dst_addr, &src_addr, trans_id, 
                  &rvals ) == OK )
      {
            if ( inet_addr( source_addr ) == inet_addr( rvals.external_ip ) )
            {
                  // Do testII
                  if ( test2( sockfd, &dst_addr, &src_addr, trans_id,
                              &rvals ) == OK )
                  {
                        // Open Internet
                        type->nat_type = OPEN_INTERNET;
                  }
                  else 
                  { 
                        // Sym. UDP Firewall
                        type->nat_type = SYM_UDP_FIREWALL;
                  }
            }
            else
            {
                  // Do TestII
                  if( test2( sockfd, &dst_addr, &src_addr, trans_id, 
                              &rvals ) == OK ) 
                  {
                        // Full Cone
                        type->nat_type = FULL_CONE;
                  }
                  else
                  {
                        if ( test1( sockfd, &dst_addr, &src_addr, trans_id, 
                                    &rvals ) == OK )
                        {
                              if ( inet_addr( source_addr ) == inet_addr( rvals.changed_ip ) )
                              {
                                    // Do testIII
                                    if (test3( sockfd, &dst_addr, &src_addr, trans_id, 
                                                &rvals ) == OK )
                                    {
                                          // Restricted
                                          type->nat_type = RESTRICTED;
                                    }
                                    else
                                    {
                                          // Port Restricted 
                                          type->nat_type = PORT_RESTRICTED;
                                    }
                              }
                              else
                              {
                                    // Symmetric NAT
                                    type->nat_type = SYM_NAT;
                              }
                        }
                        else
                        {
                              // Unexpected NAT type
                              type->nat_type = UNEXPECTED_NAT_TYPE;
                        }
                  }
            }
      }
      else 
      {     // UDP Blocked
            type->nat_type = UDP_BLOCKED;
      }

      strncpy( type->ipv4, rvals.external_ip, strlen( rvals.external_ip ) );
      type->port = rvals.external_port;

      return (sockfd);
}


int get_nat_type(char *source_addr, int source_port, 
      char *stun_server_addr, int stun_server_port,
      struct nat_type *type)
{
      if (type == NULL) return (ERROR);
      memset( type, 0, sizeof( struct nat_type ) );

      const char* _source_addr = source_addr == NULL ? DEFAULT_SRC_IP : source_addr;
      const int _source_port = source_port <= 0 ? DEFAULT_SRC_PORT: source_port;
      const int _stun_server_port = stun_server_port <= 0 ? DEFAULT_STUN_PORT : stun_server_port;

      char _stun_server_addr[18] = {0};
      if (gethostbyname1(stun_server_addr == NULL ? (char *)stun_server[0]: stun_server_addr, 
                  _stun_server_addr) != OK)
      {     
            #ifdef DEBUG
            fprintf(stderr, "[ERROR] Query DNS failed. err_code %d, err_msg: %s\n", errno, strerror(errno));
            #endif
            return (ERROR);
      }

      #ifdef DEBUG
            fprintf(stdout, "[INFO] Test NAT type %s:%d->%s:%d\n", _source_addr, _source_port, _stun_server_addr, _stun_server_port);
      #endif 
      
      return do_test(_source_addr, _source_port, _stun_server_addr, _stun_server_port, type);
}