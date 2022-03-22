
/*
  IMPORTANT.
  fgets()  same as fread but adds a terminating 0.  which means don't need CString. structure 
*/

#include "cstring.h"


#include <assert.h>



void cStringInit(CString *a, char *start_, char *end_)
{
  assert(end_ - start_ > 0); // needed for null terminal
  a->start = start_;
  a->end   = end_;


  cStringClear(a);
}



bool cStringisEmpty(const CString *a)
{
  // always true due to terminal
  return a->pos == a->start;
}

// isFull() ?


size_t cStringCount(const CString *a)
{
  // include terminal
  return a->pos - a->start;
}


size_t cStringReserve(const CString *a)
{
  return a->end - a->start;
}


static void validate(const CString *a)
{
  assert(! cStringisEmpty(a)); // never empty due to terminal
  assert(*(a->pos - 1) == 0);  // should always hold
}



int32_t cStringPeekLast(const CString *a)
{
  validate(a);
  // ie. char before the null terminal
  return *(a->pos - 2);
}


int32_t cStringPeekFirst(const CString *a)
{
  validate(a);
  return *a->start;
}


void cStringClear(CString *a)
{
  a->pos   = a->start;
  *(a->pos) = 0; // null terminal
  ++a->pos;
}


void cStringPush(CString *a, char val)
{
  validate(a);
  // check have space
  assert(a->pos != a->end);
  // overwrite terminal with the char
  *(a->pos - 1) = val;
  // set terminal
  *(a->pos) = 0;
  // advance
  ++a->pos;
  validate(a);
}



int32_t cStringPop(CString *a)
{
  validate(a);
  // get char before the null terminal
  int32_t ch = *(a->pos - 2);
  // overwrite with terminal
  *(a->pos - 2) = 0;
  // un-advance
  --a->pos;
  validate(a);
  return ch;
}



char * cStringPtr(CString *a)
{
  validate(a);
  return a->start;
}



ssize_t cStringWrite(CString *a, const char *buf, size_t size)
{
  validate(a);
  // should validate have space. or just assert error ???
  // or assert???

  size_t i = 0;

  while(i < size) {

    if(a->pos + 2 >= a->end)  // for character and terminal
      break;

    cStringPush(a, buf[i] );
    ++i;
  }

  validate(a);
  return i;
}



#if 0
#include "cstring.h"


#include <iostream>
#include <assert.h>


int main()
{
  char buf[10];

  CString s;
  cStringInit(&s, buf, buf + 10);

#if 1

  assert(cStringCount( &s) == 1);
  std::cout << "count " << cStringCount( &s) << "   string " << cStringPtr( &s) << std::endl;

  cStringWrite(&s, "hello", sizeof("hello") );
  // cStringPush(&s, 'h' );
  // cStringPush(&s, 'e' );

  std::cout << "count " << cStringCount( &s) << "   string " << cStringPtr( &s) << std::endl;


  std::cout << "last '" << ((char ) cStringPeekLast( &s) ) << "'" << std::endl;
  std::cout << "first " << ((char) cStringPeekFirst( &s) )<< std::endl;


  
  // this isn't working???
  cStringPop(&s );
  std::cout << "count " << cStringCount( &s) << "   string " << cStringPtr( &s) << std::endl;

#endif


  std::cout << "push 'h'" << std::endl;
  cStringPush(&s, 'h' );
  assert( cStringPeekLast( &s) == 'h' ); 
  assert( cStringPeekFirst( &s) == 'h' ); 
  
  std::cout << "last '" << ((char ) cStringPeekLast( &s) ) << "'" << std::endl;
  std::cout << "count " << cStringCount( &s) << "   string " << cStringPtr( &s) << std::endl;

  std::cout << "pop 'h'" << ((char)cStringPop(&s )) << std::endl;

  assert( cStringPeekLast( &s) == 'o' ); 

  std::cout << "last '" << ((char ) cStringPeekLast( &s) ) << "'" << std::endl;
  std::cout << "count " << cStringCount( &s) << "   string " << cStringPtr( &s) << std::endl;


  for(unsigned i = 0; i < 5; ++i ) {
    cStringPop(&s );
  }

  assert(cStringCount( &s) == 1);

  std::cout << "count " << cStringCount( &s) << "   string " << cStringPtr( &s) << std::endl;

  // cStringPop(&s );
}



#endif





