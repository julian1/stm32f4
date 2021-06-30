

// bit manipulation

#include <stddef.h>   // size_t
#include <stdint.h>   // uint32_t


#define MASK(width)                           ((1<<(width)) - 1)

#define SETFIELD(data, width, offset, val)    (((data) & ~(MASK(width) << (offset))) | (((val) & MASK(width)) << (offset)))

#define GETFIELD(data, width, offset)         ((data) >> (offset)) & MASK(width)


extern char * uint_to_bits(char *buf, size_t width, uint32_t value);


