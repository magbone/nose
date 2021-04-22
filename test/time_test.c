
#include "../src/nose.h"
#include "../src/timer.h"

#include <unistd.h>

struct sub_val
{
      int p;
};

struct val
{
      int a;
      void *p;
};

void * test(void *args)
{
      struct val * a = (struct val *)args;
      printf("num:%d\n", a->a++);
      printf("%p\n", a->p);
      return NULL;
}

int main()
{
      struct sub_val b = {.p = 3};
      struct val a = {.a = 0, .p = &b};
      int time_id1 = set_timeout(1, test, &a);
      printf("time_id: %d\n", time_id1);
      // int time_id2 = set_timeout(1, test, NULL);
      // printf("time_id: %d\n", time_id2);
      sleep(30);
      clear_timeout(time_id1);
      // clear_timeout(time_id2);
      return 0;
}