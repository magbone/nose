
#include "nose.h"
#include "timer.h"

#include <unistd.h>

int 
set_timeout(int sec, void* (*func)(void *args), void *args)
{
      if (t_val == NULL)
      {
            t_val = (struct time_val*)malloc(sizeof(struct time_val));
            t_val->time_id = -1;
            t_val->sec = -1;
            t_val->next = NULL;
      }

      struct time_val *new_val = (struct time_val *)malloc(sizeof(struct time_val));

      if (new_val == NULL) return (ERROR);

      struct time_val *p = t_val;
      while (p->next != NULL)
            p = p->next;
      
      new_val->next = NULL;
      int time_id = p->time_id + 1;
      new_val->time_id = time_id;
      new_val->sec = sec;
      new_val->func = func;
      new_val->args = args;
      new_val->flag = PEDDING;
      new_val->timer_thread = lanuch_task(new_val);
      p->next = new_val;
      return time_id;       
}

static pthread_t 
lanuch_task(struct time_val *val)
{
      pthread_t timer_thread;
      
      if (pthread_create(&timer_thread, NULL, call_func, val) < 0)
            return timer_thread;
      pthread_detach(timer_thread);
      // pthread_join(timer_thread, NULL);
      return timer_thread;
}


static void* 
call_func(void *val)
{
      struct time_val *t_val = (struct time_val *)val;
      t_val->flag = RUNNING;
      for (;t_val->flag == RUNNING;)
      {
            t_val->func(t_val->args);
            sleep(t_val->sec);
      }
      t_val->flag = DEAD;
      return NULL;
}


int 
clear_timeout(int time_id)
{
      if (t_val == NULL) return (ERROR);
      struct time_val *p = t_val, *q = t_val->next;

      while (q != NULL && q->next != NULL)
      {
            if (q->time_id == time_id)
                  break;
            p = p->next;
            q = q->next;
      }
      
      q->flag = KILLING;
      p->next = q->next;
      while(q->flag != DEAD);
      free(q);
      return (OK);
}