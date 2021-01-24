
#ifndef _COMMON_H_
#define _COMMON_H_
#include <uv.h>

typedef struct {
      uv_write_t req;
      uv_buf_t buf;
}write_req_t;

#endif // !_COMMON_H_