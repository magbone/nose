
#include "nose.h"
#include "dfa.h"


/*
 * @params: dfa_handler: 
 *          matrix: two-dimension matrixs for describling a series of transition functions and states.
 *  For example:
 *  int matrix[state_count + 1][state_count + 1] = {{0, s_0, s_1, s_2, s_3,}
 *                                                  {s_0, -1, c_0, -1, c_3,}
 *                                                  {s_1, c_1, -1, c_2, -1,}                         
 *                                                  {s_2, c_4, -1, c_5, -1,}                         
 *                                                  {s_3, -1,  -1, -1 ,-1 ,}}
 * The first line and first column is the name of states, other postions of matrix is a series of transition functions.
 *          m: line or column length of matrix, it must be state count + 1 and quare matrix.   
*/

int 
init_dfa(dfa* dfa_handler, int *matrix, int m, int begin_state)
{
      if (dfa_handler == NULL) return (ERROR);

      dfa_handler->matrix = malloc(sizeof(int) * m * m);
      
      // Copy the array
      for (int i = 0; i < m; i++)
            for (int j = 0; j < m; j++)
                  *(dfa_handler->matrix + i * m + j) = *(matrix + i * m + j);
      
      dfa_handler->m = m;
      dfa_handler->current_state = begin_state;
      return (OK);
}

/**
 * Get current dfa's state.
*/
int 
get_current_dfa_state(dfa *dfa_handler)
{
      if (dfa_handler == NULL) return (ERROR);
      return dfa_handler->current_state;
}

/**
 * 
 * Update the dfa's state when the condition is changed.
*/
int 
set_next_dfa_state(dfa *dfa_handler, int condition)
{
      if (dfa_handler == NULL) return (ERROR);

      // Column first
      for (int col = 1; col < dfa_handler->m; col++)
      {
            if (*(dfa_handler->matrix + 0 * dfa_handler->m + col) == dfa_handler->current_state)
            {
                  for(int line = 1; line < dfa_handler->m; line++)
                  {
                        if (*(dfa_handler->matrix + line * dfa_handler->m + col) == condition)
                        {
                              int old_state = dfa_handler->current_state;
                              dfa_handler->current_state = *(dfa_handler->matrix + line * dfa_handler->m + 0);

                              if (dfa_handler->call_back != NULL)
                                    dfa_handler->call_back(old_state, dfa_handler->current_state, condition, dfa_handler->cb_arg);

                              return (OK); 
                        }
                  }
            }
      }
      // Line first
      for (int line = 1; line < dfa_handler->m; line++)
      {
            if (*(dfa_handler->matrix + line * dfa_handler->m + 0) == dfa_handler->current_state)
            {
                  for(int col = 1; col < dfa_handler->m; col++)
                  {
                        if (*(dfa_handler->matrix + line * dfa_handler->m + col) == condition)
                        {
                              int old_state = dfa_handler->current_state;
                              dfa_handler->current_state = *(dfa_handler->matrix + 0 * dfa_handler->m + col);

                              if (dfa_handler->call_back != NULL)
                                    dfa_handler->call_back(old_state, dfa_handler->current_state, condition, dfa_handler->cb_arg);
                              
                              return (OK); 
                        }
                  }
            }
      }

      return (ERROR);
}


/**
 * Register a listener for state when it be changed.
 * Call it before set_next_dfa_state() is called.
*/
void 
dfa_state_change_listener(dfa *dfa_handler, void (*cb )(int before, int after, int condition, void* arg), void* arg)
{
      if (cb == NULL || dfa_handler == NULL) return;
      dfa_handler->call_back = cb;
      dfa_handler->cb_arg = arg;
}