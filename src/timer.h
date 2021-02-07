#ifndef _TIMER_H_
#define _TIMER_H_

struct time_val{
      int time_id;
      struct time_val *next;
};

static struct time_val *t_val;

int set_timeout(void (*func)(void *args), void *args);

int clear_timeout(int time_id);

#endif // !_TIMER_H_