
#include "nose.h"
#include "timer.h"


int set_timeout(void (*func)(void *args), void *args)
{
      if (t_val == NULL)
      {
            t_val = (struct time_val*)malloc(sizeof(struct time_val));
            t_val->time_id = -1;
            t_val->next = NULL;
      }
      return 0;       
}
