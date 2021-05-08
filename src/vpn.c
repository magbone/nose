
#include "nose.h"
#include "vpn.h"
#include "auth.h"
#include <time.h>

static void 
state_cb(int before, int after, int condition, void* arg)
{

}

int
init_vpn(struct vpn *handler, char *pre_key, void (*state_change_cb)(char *buf, size_t len))
{
      if (handler == NULL || pre_key == NULL) return (ERROR);

      strncpy(handler->pre_key, pre_key, 32);

      init_dfa(&handler->v_dfa, *state_matrix, STATE_M, S_START);  

      if (state_change_cb != NULL)
            handler->state_change_cb = state_change_cb;
      dfa_state_change_listener(&handler->v_dfa, state_cb, handler);
      
      srand((unsigned)time(NULL));
      handler->id = rand() % 32768;
      return (OK);   
}

int 
vpn_processing(struct vpn *handler)
{
      if (handler == NULL) return (ERROR);
      char buf[BUFSIZ];
      int len;
      switch (get_current_dfa_state(&handler->v_dfa))
      {
      case S_START:
            handler->challenge = rand() % 32768;
            if ((len = auth_request_pkt(handler->id, handler->challenge, handler->pre_key, buf)) < 0)
            {
                  fprintf(stderr, "[ERROR] (VPN) auth_request_pkt\n");
                  return (ERROR);
            }
            break;
      case S_AUTHEN:
            
      default:
            break;
      }
      return (OK);
}