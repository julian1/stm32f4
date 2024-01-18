
#ifdef __cplusplus
extern "C" {
#endif



// bit manipulation


#define MASK(width)                           ((1<<(width)) - 1)

#define SETFIELD(data, width, offset, val)    (((data) & ~(MASK(width) << (offset))) | (((val) & MASK(width)) << (offset)))

#define GETFIELD(data, width, offset)         ((data) >> (offset)) & MASK(width)


#ifdef __cplusplus
}
#endif



