#ifndef _BUCKET_H_
#define _BUCKET_H_

#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define PALIVE     1
#define PDEAD      2
#define PUNKOWN    3

struct bucket_item
{
      uint16_t port;
      char ipv4[16];
      char node_id[21];

      int state;
      time_t join_timestap; 

      // Only for peer
      char vlan_ipv4[16]; 
      int nat_type;
      
};

#define MAX_BUCKET_SIZE 255

struct bucket
{
      int top, visited;
      struct bucket_item b[MAX_BUCKET_SIZE];
};



static pthread_mutex_t mutex;

void init_bucket(struct bucket *bkt, struct bucket_item *init_items, int init_items_size);

void push_front_bucket(struct bucket *bkt, struct bucket_item item);

struct bucket_item *get_front_bucket(struct bucket *bkt);

struct bucket_item *get_back_bucket(struct bucket *bkt);

struct bucket_item *get_next_bucket_item(struct bucket *bkt);

int get_top_bucket_items(struct bucket *bkt, struct bucket_item *item, int size);

void pop_front_bucket(struct bucket *bkt);

void pop_back_bucket(struct bucket *bkt);

int is_empty_bucket(struct bucket *bkt);

int bucket_size(struct bucket *bkt);

int get_item_by_vlan_ipv4(struct bucket *bkt, char *vlan_ipv4, struct bucket_item *item);

void destory_bucket(struct bucket *bkt);

#endif // !_BUCKET_H_