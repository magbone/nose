

#ifndef _DFA_H_
#define _DFA_H_

typedef struct 
{
      int *matrix;
      int m;
      int current_state;

      void (*call_back) (int before, int atfer, int condition, void *arg);
      void *cb_arg;
}dfa;


int init_dfa(dfa* dfa_handler, const int *matrix, int m, int begin_state);

int get_current_dfa_state(dfa *dfa_handler);

int set_next_dfa_state(dfa *handler, int condition);

void dfa_state_change_listener(dfa *dfa_handler, void (*cb )(int before, int after, int condition, void* arg), void* arg);

#endif // !_DFA_H_