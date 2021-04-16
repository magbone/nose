


#include "nose.h"
#include "bucket.h"

#include <string.h>

void 
init_bucket(struct bucket *bkt, struct bucket_item *init_items, int init_items_size)
{
      bkt->top = -1;
      bkt->visited = 0;

      if (bkt == NULL || init_items == NULL) return;

      pthread_mutex_init(&mutex, NULL);

      if (init_items_size > MAX_BUCKET_SIZE) return;

      for (int i = 0; i < init_items_size; i++)
      {
             bkt->b[i] = init_items + i;
             bkt->top++;
      }
}

void 
push_front_bucket(struct bucket *bkt, struct bucket_item *item)
{
      if (bkt == NULL) return;

      pthread_mutex_lock(&mutex);

      for (int i = 0; i <= bkt->top; i++)
      {
            if (strcmp(item->node_id, bkt->b[i]->node_id) == 0) 
            {
                  pthread_mutex_unlock(&mutex);
                  return;
            }
            
      }
      if (bkt->top + 1 == MAX_BUCKET_SIZE)
            pop_back_bucket(bkt);

      bkt->b[++bkt->top] = item;

      pthread_mutex_unlock(&mutex);
}

int 
get_top_bucket_items(struct bucket *bkt, struct bucket_item *item, int size)
{
      if (bkt == NULL || item == NULL) return 0;

      int r_size = 0;

      pthread_mutex_lock(&mutex);
      r_size = size > 1 + bkt->top ? bkt->top + 1: size;

      for (int i = bkt->top, j = 0; i >= 0; i--, j++)
            *(item + j) = *(*(bkt->b) + i);
      pthread_mutex_unlock(&mutex);

      return r_size;
}

struct bucket_item *
get_front_bucket(struct bucket *bkt)
{
      pthread_mutex_lock(&mutex);
      struct bucket_item *item = bkt->top == -1 ? NULL : *(bkt->b) + bkt->top;
      pthread_mutex_unlock(&mutex);
      return item;
}

struct bucket_item *
get_back_bucket(struct bucket *bkt)
{
      pthread_mutex_lock(&mutex);
      struct bucket_item *item = *(bkt->b);
      pthread_mutex_unlock(&mutex);
      return item;
}

struct bucket_item *
get_next_bucket_item(struct bucket *bkt)
{

      pthread_mutex_lock(&mutex);
      struct bucket_item *item = NULL;

      bkt->visited %= (bkt->top + 1);
      item = bkt->b[bkt->visited++]; 

      pthread_mutex_unlock(&mutex);
      return item;
}


void 
pop_front_bucket(struct bucket *bkt)
{
      pthread_mutex_lock(&mutex);
      if (bkt->top >= 0)
            bkt->b[bkt->top--] = NULL;
      pthread_mutex_unlock(&mutex);
}

int 
is_empty_bucket(struct bucket *bkt)
{
      pthread_mutex_lock(&mutex);
      int is = bkt->top == -1;
      pthread_mutex_unlock(&mutex);
      return is;
}


void 
pop_back_bucket(struct bucket *bkt)
{
      pthread_mutex_lock(&mutex);
      for (int i = 1; i <= bkt->top; i++)
            *(bkt->b + i - 1) = *(bkt->b + i);
      
      bkt->top--;
      pthread_mutex_lock(&mutex);
}

void 
destory_bucket(struct bucket *bkt)
{
      pthread_mutex_destroy(&mutex);
}