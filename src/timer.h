#ifndef _TIMER_H_
#define _TIMER_H_

#include <pthread.h>

#define PEDDING   0
#define RUNNING   1
#define KILLING   2
#define DEAD      3
struct time_val{
      int time_id;
      int sec;
      int flag;
      pthread_t timer_thread;
      void *(*func)(void *);
      void *args;
      struct time_val *next;
};

static struct time_val *t_val;

int set_timeout(int sec, void * (*func)(void *args), void *args);

int clear_timeout(int time_id);

static pthread_t lanuch_task(struct time_val *val);

static void* call_func(void *val);

#endif // !_TIMER_H_