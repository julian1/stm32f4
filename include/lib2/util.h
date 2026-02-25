/*

  now mostly only prototypes, for code that has been moved elsewhere
  mostly into project specific

*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif





#define UNUSED(x) ((void)(x))

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))





#ifdef __cplusplus
}
#endif

