

#include"utils.h"



u_int16_t 
ltobs(u_int16_t u)
{
      return (((u << 8) | (u >> 8)) & 0xffff);
}


