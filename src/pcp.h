#ifndef _PCP_H_
#define _PCP_H_

#include <stdint.h>

/**
 * Peer Control Protocol
 * 
 * 
*/


// Definitions of flags field of PCP structure

#define F_HELLO_ACK 1
#define F_HELLO_SYN 2
#define F_HB_ACK    3
#define F_HB_SYN    4
#define F_PAYLOAD   5

struct PCP
{
      uint8_t ver; // Must be zero;
      uint8_t flags; 
      uint16_t len; // Payload length
};

int PCP_hello_syn(char *buf);

int PCP_hello_ack(char *buf);

int PCP_hb_syn(char *buf);

int PCP_hb_ack(char *buf);

int PCP_payload_pkt(char *buf, char *bbuf, int all_size, uint16_t text_size);

#endif // !_PCP_H_