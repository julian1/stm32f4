/*
  rename common.h, support.h ?

*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif





#define UNUSED(x) ((void)(x))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

// https://stackoverflow.com/questions/19452971/array-size-macro-that-rejects-pointers
/*
#define ARRAY_SIZE(arr) \
    (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

#define __must_be_array(a) BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))
#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))

*/


#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))



#ifdef __cplusplus
}
#endif

