


#include "nose.h"
#include "bucket.h"

void 
init_bucket(struct bucket *bkt, struct bucket_item *init_items, int init_items_size)
{
      if (bkt == NULL || init_items == NULL) return;

      pthread_mutex_init(&mutex, NULL);

      bkt->b = (struct bucket_item *)malloc(sizeof(struct bucket_item) * MAX_BUCKET_SIZE);
      if (bkt->b == NULL) return;

      bkt->top = -1;

      if (init_items_size > MAX_BUCKET_SIZE) return;

      for (int i = 0; i < init_items_size; i++)
            *(bkt->b + i) = *(init_items + i), bkt->top++;

      
}

void 
push_front_bucket(struct bucket *bkt, struct bucket_item item)
{
      if (bkt == NULL) return;

      pthread_mutex_lock(&mutex);
      if (bkt->top + 1 == MAX_BUCKET_SIZE)
            pop_back_bucket(bkt);

      *(bkt->b + ++bkt->top) = item;
      pthread_mutex_unlock(&mutex);
}


struct bucket_item 
get_front_bucket(struct bucket *bkt)
{
      pthread_mutex_lock(&mutex);
      struct bucket_item item = *(bkt->b + bkt->top);
      pthread_mutex_unlock(&mutex);
      return item;
}

struct bucket_item
get_back_bucket(struct bucket *bkt)
{
      pthread_mutex_lock(&mutex);
      struct bucket_item item = *(bkt->b);
      pthread_mutex_unlock(&mutex);
      return item;
}

void 
pop_front_bucket(struct bucket *bkt)
{
      pthread_mutex_lock(&mutex);
      bkt->top--;
      pthread_mutex_unlock(&mutex);
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
      if(bkt->b) free(bkt->b), bkt->top = -1;
      pthread_mutex_destroy(&mutex);
}