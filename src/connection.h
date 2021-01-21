
#include "protocols.h"

#define S_SCCRQ SCCRQ
#define S_SCCRP SCCRP
#define S_OCRQ  OCRQ
#define S_OCRP  OCRP
#define S_SLI   SLI 

#define LIVING_LOOP 20
#define ERROR_STATE -1

#define CALL_ID 996


struct vpn_peer{
      u_int16_t call_id;
      u_int16_t peer_id;
};


int make_socket(char *dst_addr, int port);

int mainloop(int device_fd, int connnection_fd);

static int make_raw_socket(char *dst_addr, int protocol);

static void do_PPTP_connection(int connection_fd, struct vpn_peer *peer);

static void do_PPP_connection(int connection_fd, struct vpn_peer *peer);

static int fill_SCCRq_packet(char *buffer, int len, char *host_name, char *vendor_name);

static int fill_OCRq_packet(char *buffer, int len, char *phone_number, char *subaddr);

static int fill_SLI_packet(char *buffer, int len, u_int16_t peer_call_id);

static int send_packet(char *buffer, int sock_fd, int len);

static int read_buffer(char *buffer, int sock_fd, int len);

static int clip_OCRp_packet(char *buffer, struct vpn_peer *peer);