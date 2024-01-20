
#pragma once




#include <stdint.h>




typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

#define unlikely(x)     __builtin_expect(!!(x), 0)

#define noinline


// JA
#define STATIC static
#define INIT



/*
  https://fossd.anu.edu.au/linux/latest/source/tools/include/tools/be_byteshift.h
  https://fossd.anu.edu.au/linux/latest/source/tools/include/tools/le_byteshift.h
*/

static inline uint16_t get_unaligned_be16(const uint8_t *p)
{
  return p[0] << 8 | p[1];
}

static inline uint32_t get_unaligned_be32(const uint8_t *p)
{
  return p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
}




static inline uint32_t get_unaligned_le32(const uint8_t *p)
{
  return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}


/*
  https://github.com/smx-smx/asuswrt-rt/blob/master/apps/public/dproxy-nexgen/dns_decode.c
*/


#define get_unaligned(ptr) \
  ({ __typeof__(*(ptr)) __tmp; memmove(&__tmp, (ptr), sizeof(*(ptr))); __tmp; })

#define put_unaligned(val, ptr)    \
  ({ __typeof__(*(ptr)) __tmp = (val);   \
     memmove((ptr), &__tmp, sizeof(*(ptr)));  \
     (void)0; })


#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))



