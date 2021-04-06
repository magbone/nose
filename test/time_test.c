
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
      int time_id1 = set_timeout(1, test, NULL);
      printf("time_id: %d\n", time_id1);
      int time_id2 = set_timeout(1, test, NULL);
      printf("time_id: %d\n", time_id2);
      sleep(30);
      clear_timeout(time_id1);
      clear_timeout(time_id2);
      return 0;
}