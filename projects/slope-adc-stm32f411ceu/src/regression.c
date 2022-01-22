
#undef DEBUG



#include "assert.h"   // required.
#include <matrix.h>


/* bd_free -- frees BAND matrix -- returns (-1) on error and 0 otherwise */
int bd_free(BAND *A)
{
  assert(0);
}

typedef struct ITER ITER;

/* iter_free - release memory */
int iter_free(ITER *ip)
{
  assert(0);
}

typedef struct SPROW  SPROW;
int sprow_free(SPROW *r)
{
  assert(0);
}

/* sp_free -- frees up the memory for a sparse matrix */
typedef struct SPMAT SPMAT;
int sp_free(SPMAT *A)
{
  assert(0);
}

typedef struct ZVEC ZVEC;
int zv_free(ZVEC *vec)
{
  assert(0);
}


typedef struct ZMAT ZMAT;
int zm_free(ZMAT *mat)
{
  assert(0);
}

#if 1

int	ev_err(const char *file, int err_num, int line_num,
	       const char *fn_name, int list_num)
{

  /* see implementation in err.c 
    it calls exit() which is problematic in exposing unlinked __fini symbol in embedded environment. 
    and also calls fprintf
  */

}

#endif



void func()
{
  /*
    loop() subsumes update()
  */


    MAT   *A = m_get(3,4);

}
