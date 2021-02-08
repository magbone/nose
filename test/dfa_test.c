
#include "../src/nose.h"
#include "../src/dfa.h"
#include "../src/dfa_state.h"

void call_back(int before, int after, int condition, void *args)
{
      printf("%s---%s---->%s\n", S_STR[before], C_STR[condition], S_STR[after]);
}

int main()
{
      dfa * handler = (dfa *)malloc(sizeof(dfa));
      if (handler == NULL) return 0;

      init_dfa(handler, *state_matrix, STATE_M, S_START);
      dfa_state_change_listener(handler, call_back, NULL);
      
      set_next_dfa_state(handler, C_START_AUTHEN);
      set_next_dfa_state(handler, C_AUTHEN_FAILED);
      set_next_dfa_state(handler, C_START_AUTHEN);
      set_next_dfa_state(handler, C_START_KEYEXC);
      set_next_dfa_state(handler, C_KEY_EXEC_FAILED);
      set_next_dfa_state(handler, C_START_KEYEXC);
      set_next_dfa_state(handler, C_KEY_EXEC_SUCCESS);
      set_next_dfa_state(handler, C_SEND_PAY_LOAD);
      set_next_dfa_state(handler, C_SEND_PAY_LOAD);
      free(handler);
      return 0;
}