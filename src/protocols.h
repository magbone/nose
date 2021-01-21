
#include "nose.h"

#define MAGIC_COOKIE 0x1A2B3C4D

#define PPTP_VER 1 // define the PPTP veriosn as 1

/*
      *
      * 
      * Control Message                        Message Code
      *(Control Connection Management)
      * Start-Control-Connection-Request       1
      * Start-Control-Connection-Reply         2
      * Stop-Control-Connection-Request        3
      * Stop-Control-Connection-Reply          4
      * Echo-Request                           5
      * Echo-Reply                             6
      * 
      * (Call Management)
      * Outgoing-Call-Request                  7
      * Outgoing-Call-Reply                    8
      * Incoming-Call-Request                  9
      * Incoming-Call-Reply                    10
      * Incoming-Call-Connected                11
      * Call-Clear-Request                     12
      * Call-Disconnect-Notify                 13
      * 
      * (Error Reporting)
      * WAN-Error-Notify                       14
      * 
      * (PPP Session Control)
      * Set-Link-Info                          15 
      */

#define SCCRQ     1
#define SCCRP     2
#define STCCRQ    3
#define STCCRP    4
#define ERQ       5
#define ERP       6
#define OCRQ      7
#define OCRP      8
#define SLI       15



struct PPTP_shared_header{
      // PPTP packet length
      u_int16_t length;
      // 1 for control message
      u_int16_t message_type;
      u_int32_t magic_cookie;

      
      u_int16_t ctrl_message_type;
      u_int16_t reserved0;
};


// PPTP start control connection request
struct PPTP_SCCRq{
      u_int16_t length;
      u_int16_t message_type;
      u_int32_t magic_cookie;
      u_int16_t ctrl_message_type;
      u_int16_t reserved0;
      u_int16_t protocol_version;
      u_int16_t reserved1;
      u_int32_t framing_cap;
      u_int32_t bearer_cap;
      u_int16_t max_channels;
      u_int16_t firmware_revision;
      char host_name[64];
      char vender_name[64];

};

// PPTP start control connection reply
struct PPTP_SCCRp{
      
      u_int16_t length;
      u_int16_t message_type;
      u_int32_t magic_cookie;
      u_int16_t ctrl_message_type;
      u_int16_t reserved0;
      u_int16_t protocol_version;
      u_int16_t reserved1;
      u_int32_t framing_cap;
      u_int32_t bearer_cap;
      u_int16_t max_channels;
      u_int16_t firemware_revision;
      char host_name[64];
      char vender_name[64];

};

// PPTP stop control connection request
struct PPTP_StCCRq{
      struct PPTP_shared_header header;
      u_int8_t reason;
      u_int8_t reserved1;
      u_int16_t reserved0;
};

// PPTP stop control connection reply
struct PPTP_StCCRP{
      struct PPTP_shared_header header;
      u_int8_t result_code;
      u_int8_t error_code;
      u_int16_t reserved1;
};


// PPTP echo request
struct PPTP_ERq{
      struct PPTP_shared_header header;
      u_int32_t identifier;
};

// PPTP echo reply
struct PPTP_ERp{
      struct PPTP_shared_header header;
      u_int32_t identifier;
      u_int8_t result_code;
      u_int8_t error_code;
      u_int16_t reserved1;
};

// PPTP outgoing call request

struct PPTP_OCRq{
      u_int16_t length;
      u_int16_t message_type;
      u_int32_t magic_cookie;
      u_int16_t ctrl_message_type;
      u_int16_t reserved0;
      u_int16_t call_id;
      u_int16_t call_serial_number;
      u_int32_t min_BPS;
      u_int32_t max_BPS;
      u_int32_t bearer_type;
      u_int32_t framing_type;
      union{
            u_int16_t packet_recv;
            u_int16_t window_size;
      };

      u_int16_t packet_prcessing_delay;
      u_int16_t phone_number_len;
      u_int16_t reserved1;
      char phone_number[64];
      char subaddr[64];
};

// PPTP outgoing reply; 
struct PPTP_OCRp{
      u_int16_t length;
      u_int16_t message_type;
      u_int32_t magic_cookie;
      u_int16_t ctrl_message_type;
      u_int16_t reserved0;
      u_int16_t call_id;
      u_int16_t peer_call_id;
      u_int8_t result_code;
      u_int8_t error_code;
      u_int16_t cause_code;
      u_int32_t connect_speed;
      union{
            u_int16_t packet_recv;
            u_int16_t window_size;
      };

      u_int16_t packet_prcessing_deply;
      u_int32_t physical_channel_id;
};

struct PPTP_SLI{
      u_int16_t length;
      u_int16_t message_type;
      u_int32_t magic_cookie;
      u_int16_t ctrl_message_type;
      u_int16_t reserved0;
      u_int16_t peer_call_id;
      u_int16_t reserved1;
      u_int32_t send_accm;
      u_int32_t receive_accm;
};

// Enhanced GRE header 
struct EGRE_header{

#if BYTE_ORDER == LITTLE_ENDIAN
      u_int16_t s_source_route:1;
      u_int16_t recur:3;
      u_int16_t seq_number:1;
      u_int16_t key:1;
      u_int16_t routing:1;     
      u_int16_t checksum:1;    
#elif BYTE_ORDER == BIG_ENDIAN     
      u_int16_t checksum:1;
      u_int16_t routing:1;
      u_int16_t key:1;
      u_int16_t seq_number:1;
      u_int16_t s_source_route:1;
      u_int16_t recur:3;
#endif
       

#if BYTE_ORDER == LITTLE_ENDIAN
      u_int16_t ver:3;
      u_int16_t ack:1;
      u_int16_t flags:4;
#elif BYTE_ORDER == BIG_ENDIAN
      u_int16_t ack:1;
      u_int16_t flags:4;
      u_int16_t ver:3;
#endif
      
      u_int16_t protocol_type;
      u_int16_t key_payload_len;
      u_int16_t key_call_id;

};

#define LCP   0xc021 // Link Control Protocol 
#define PAP   0xc023 // Password Authentication Protocol
#define LQR   0xc025 // Link Quality Report
#define CHAP  0xc223 // Change Handshake Authentication Protocol

struct PPP
{
      u_int8_t address;
      u_int8_t control;
      u_int16_t protocol;
};

#define CONF_REQ 1 // Configure-Rquest
#define CONF_ACK 2 // Configure-Ack
#define CONF_NAK 3 // Configure-Nak
#define CONF_REJ 4 // Configure-Reject
#define TERM_REQ 5 // Terminate-Request
#define TERM_ACK 6 // Terminate-Ack
#define CODE_REJ 7 // Code-Reject
#define PROT_REJ 8 // Protocol-Reject
#define ECHO_REQ 9 // Echo-Request
#define ECHO_REP 10 // Echo-Reply
#define DISC_REQ 11  //Discard-Request

struct PPP_LCP
{
      u_int8_t code;
      u_int8_t indentfier;
      u_int16_t length;
};

struct ip* filling_IP_header(char *dst, int protocol, int payload); 

struct EGRE_header* filling_EGRE_header(int key_payload_len, int call_id);

struct PPP *filling_PPP(int protocol);

struct PPP_LCP *filling_PPP_LCP(int code, int id, int len);
