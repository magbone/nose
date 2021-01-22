
#include "nose.h"


struct node{
      char *data;
      struct node *next;
};

struct queue
{
      int len;
      struct node *head;
      struct node *tail;
};

struct queue * init_new_queue();

void enqueue(struct queue *q, char *);

char *dequeue(struct queue *q);

