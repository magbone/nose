#ifndef _TIMER_H_
#define _TIMER_H_

#include <pthread.h>

// Task state

#define PEDDING   0
#define RUNNING   1
#define KILLING   2
#define DEAD      3

struct time_val{
      int time_id;
      int sec;
      int flag;  // The state of current 
      pthread_t timer_thread;
      void *(*func)(void *); // Callback function
      void *args; // The argument passing to function
      struct time_val *next;
};

static struct time_val *t_val;

static int terminated_handled = 0;

int set_timeout(int sec, void * (*func)(void *args), void *args);

int clear_timeout(int time_id);

static pthread_t lanuch_task(struct time_val *val);

static void* call_func(void *val);

static void handle_sig(int sig);

#endif // !_TIMER_H_