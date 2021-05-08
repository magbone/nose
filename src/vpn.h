
#ifndef _VPN_H_
#define _VPN_H_

#include "dfa.h"
#include "dfa_state.h"

struct vpn
{
      char pre_key[32];
      dfa v_dfa;
      void (*state_change_cb)(char *buf, size_t len);
      int id;
      int challenge;
};

static void state_cb(int before, int after, int condition, void* arg);

int init_vpn(struct vpn *handler, char *pre_key, void (*state_change_cb)(char *buf, size_t len));

int vpn_processing(struct vpn *handler);

#endif // !_VPN_H_