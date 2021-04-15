#ifndef _BUCKET_H_
#define _BUCKET_H_

#include <stdint.h>
#include <pthread.h>

struct bucket_item
{
      u_int16_t port;
      char ipv4[16];
      char node_id[21];
      int time_id;
      int discovery_time;

      int nat_type; // Only for peer
};

#define MAX_BUCKET_SIZE 255

struct bucket
{
      int top;
      struct bucket_item *b[MAX_BUCKET_SIZE];
};



static pthread_mutex_t mutex;

void init_bucket(struct bucket *bkt, struct bucket_item *init_items, int init_items_size);

void push_front_bucket(struct bucket *bkt, struct bucket_item *item);

struct bucket_item *get_front_bucket(struct bucket *bkt);

struct bucket_item *get_back_bucket(struct bucket *bkt);

int get_top_bucket_items(struct bucket *bkt, struct bucket_item *item, int size);

void pop_front_bucket(struct bucket *bkt);

void pop_back_bucket(struct bucket *bkt);

int is_empty_bucket(struct bucket *bkt);

void destory_bucket(struct bucket *bkt);


#endif // !_BUCKET_H_