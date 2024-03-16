
/*
// Note. also over-ridden c functions in util.c

  including this fixes up _fini link error.
*/



#include <unistd.h>   // isatty()

#include <assert.h>   // required.
#include <mesch12b/matrix.h>

// JA we need stub functions for these, because we don't compile them



#define UNUSED(x) (void)(x)

typedef struct ITER ITER;
typedef struct SPROW  SPROW;
typedef struct SPMAT SPMAT;
typedef struct ZVEC ZVEC;
typedef struct ZMAT ZMAT;


// int bd_free(BAND *A);
int iter_free(ITER *ip);
int sprow_free(SPROW *r);
int sp_free(SPMAT *A);
int zv_free(ZVEC *vec);
int zm_free(ZMAT *mat);




/* bd_free -- frees BAND matrix -- returns (-1) on error and 0 otherwise */
int bd_free(BAND *A)
{
  UNUSED(A);
  assert(0);
  return 0;
}


/* iter_free - release memory */
int iter_free(ITER *ip)
{
  UNUSED(ip);
  assert(0);
  return 0;
}

int sprow_free(SPROW *r)
{
  UNUSED(r);
  assert(0);
  return 0;
}

/* sp_free -- frees up the memory for a sparse matrix */
int sp_free(SPMAT *A)
{
  UNUSED(A);
  assert(0);
  return 0;
}

int zv_free(ZVEC *vec)
{
  UNUSED(vec);
  assert(0);
  return 0;
}


int zm_free(ZMAT *mat)
{
  UNUSED(mat);
  assert(0);
  return 0;
}


#if 0
int fileno(void *p) 
{
    UNUSED(p);
    return 0;
}
#endif

#if 1
int isatty(int x) 
{
    UNUSED(x);
    return 0;
}
#endif




void exit(int status) 
{
  // must intercept, to avoid link errors via linking standard library
  UNUSED(status);

  // suppress error about function marked no return, returning
  for(;;);
}


/*
  EXTR.

  we override printf and fprintf in util.c
  so they take preference in the linking. 
*/







#if 0

int	ev_err(const char *file, int err_num, int line_num,
	       const char *fn_name, int list_num)
{
  /* see implementation in err.c 
    it calls exit() which is problematic in exposing unlinked __fini symbol in embedded environment. 
    and also calls fprintf
  */


  UNUSED(file);
  UNUSED(err_num);
  UNUSED(line_num);
  UNUSED(fn_name);
  UNUSED(list_num);


}

#endif






