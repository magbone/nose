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

// 对桶进行初试化操作，可以事先指定桶中的条目。
void init_bucket(struct bucket *bkt, struct bucket_item *init_items, int init_items_size);
// 对指定桶添加一个条目，并且该条目处于桶的最上层。
void push_front_bucket(struct bucket *bkt, struct bucket_item item);
// 获取指定桶中最上层条目。
struct bucket_item *get_front_bucket(struct bucket *bkt);
// 获取制定桶中最底层条目。
struct bucket_item *get_back_bucket(struct bucket *bkt);
// 在循环迭代迭代下，获取下一个条目。
struct bucket_item *get_next_bucket_item(struct bucket *bkt);
// 获取指定桶中给定数目的上层条目集。
int get_top_bucket_items(struct bucket *bkt, struct bucket_item *item, int size);
// 删除指定桶中最顶层条目，与get_为前缀的函数相比，该函数不会返回顶层条目，而后者不会删除顶层条目。
void pop_front_bucket(struct bucket *bkt);
// 弹出指定桶中最底层条目。
void pop_back_bucket(struct bucket *bkt);
// 指定桶是否为空，为空返回1，否则返回0。
int is_empty_bucket(struct bucket *bkt);
// 返回给定桶的大小，即内含条目的数目。
int bucket_size(struct bucket *bkt);
// 获取到查找到的第一个符合某个虚拟IP地址的条目。
int get_item_by_vlan_ipv4(struct bucket *bkt, char *vlan_ipv4, struct bucket_item *item);
// 将某个结点移动到桶的底部
int bucket_move_to_bottom( struct bucket *bkt, char *node_id );
// 销毁指定的桶，清空里面的条目。
void destory_bucket(struct bucket *bkt);

#endif // !_BUCKET_H_