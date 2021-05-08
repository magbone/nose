

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "nose.h"

// Definition of type
#define AUTH      0 // Autheniction
#define KEY_NEG   1 // Key Negotiation
#define APP_DATA  2 // Application Data

// Definition of code for Authenication
#define AUTH_REQ 0
#define AUTH_RSP 1


struct vpn_proto_header
{
      u_int8_t type;
      u_int8_t code;
      u_int16_t id;
      u_int16_t length;
      u_int16_t reversed;
};

#endif // !_PROTOCOL_H_
