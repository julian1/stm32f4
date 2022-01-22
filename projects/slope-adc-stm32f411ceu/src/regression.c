
#undef DEBUG



#include "regression.h"
// #include "assert.h"   // required.
#include <matrix.h>



void regression(void)
{
  /*
    loop() subsumes update()
  */

    printf("WHOOT in regression()\n");

    MAT   *A = m_get(3,4);

    m_foutput(stdout, A );

    M_FREE(A);
}





