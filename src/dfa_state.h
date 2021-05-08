
#ifndef _DFA_STATE_H
#define _DFA_STATE_H

#include "nose.h"

#define STATE_M 5

#define S_START   1
#define S_AUTHEN  2
#define S_KEY_EX  3
#define S_RUN     4

#define C_START_AUTHEN        1
#define C_AUTHEN_FAILED       2
#define C_START_KEYEXC        3
#define C_KEY_EXEC_FAILED     4
#define C_KEY_EXEC_SUCCESS    5
#define C_SEND_PAY_LOAD       6

const char *S_STR[] = {
      NULL,
      "Start",
      "Authentication",
      "Key exchange",
      "Run",
      NULL
};

const char *C_STR[] = {
      NULL,
      "Start authenticating",
      "Authentication failed or tiemout",
      "Start key exchange",
      "Key exchange failed or timeout",
      "Key echange success",
      "Send the payload",
      NULL
};

const int state_matrix[STATE_M][STATE_M] = {{0       , S_START        , S_AUTHEN         , S_KEY_EX      , S_RUN             }, 
                                            {S_START , 0              , C_START_AUTHEN   , 0             , 0                 },
                                            {S_AUTHEN, C_AUTHEN_FAILED, 0                , C_START_KEYEXC, 0                 },
                                            {S_KEY_EX, 0              , C_KEY_EXEC_FAILED, 0             , C_KEY_EXEC_SUCCESS},
                                            {S_RUN   , 0              , 0                , 0             , C_SEND_PAY_LOAD   }};

#endif // !_DFA_STATE_H