
#include "../src/nose.h"
#include "../src/timer.h"

#include <unistd.h>

void * test(void *args)
{
      static int i = 0;
      printf("num:%d\n", i++);
      return NULL;
}

int main()
{
      int time_id = set_timeout(1, test, NULL);
      printf("time_id: %d\n", time_id);
      sleep(10);
      clear_timeout(time_id);
      time_id = set_timeout(1, test, NULL);
      printf("time_id: %d\n", time_id);
      sleep(10);
      clear_timeout(time_id);
      return 0;
}