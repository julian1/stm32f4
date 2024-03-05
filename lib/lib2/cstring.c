
/*
  IMPORTANT.
  fgets()  same as fread but adds a terminating 0.  which means don't need cstring_t. structure 
*/

#include "cstring.h"


#include <assert.h>



void cstring_init(cstring_t *a, char *start_, char *end_)
{
  assert(end_ - start_ > 0); // needed for null terminal
  a->start = start_;
  a->end   = end_;


  cstring_clear(a);
}



bool cstring_empty(const cstring_t *a)
{
  // always true due to terminal
  return a->pos == a->start;
}

// isFull() ?


size_t cstring_count(const cstring_t *a)
{
  // include terminal
  return a->pos - a->start;
}


size_t cstring_reserve(const cstring_t *a)
{
  return a->end - a->start;
}


static void validate(const cstring_t *a)
{
  assert(! cstring_empty(a)); // never empty due to terminal
  assert(*(a->pos - 1) == 0);  // should always hold
}



int32_t cstring_peek_last(const cstring_t *a)
{
  validate(a);
  // ie. char before the null terminal
  return *(a->pos - 2);
}


int32_t cstring_peek_first(const cstring_t *a)
{
  validate(a);
  return *a->start;
}


void cstring_clear(cstring_t *a)
{
  a->pos   = a->start;
  *(a->pos) = 0; // null terminal
  ++a->pos;
}


void cstring_push_back(cstring_t *a, char val)
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



int32_t cstring_pop_back(cstring_t *a)
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



char * cstring_ptr(cstring_t *a)
{
  validate(a);
  return a->start;
}



ssize_t cstring_write(cstring_t *a, const char *buf, size_t size)
{
  validate(a);
  // should validate have space. or just assert error ???
  // or assert???

  size_t i = 0;

  while(i < size) {

    if(a->pos + 2 >= a->end)  // for character and terminal
      break;

    cstring_push_back(a, buf[i] );
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

  cstring_t s;
  cstring_init(&s, buf, buf + 10);

#if 1

  assert(cstring_count( &s) == 1);
  std::cout << "count " << cstring_count( &s) << "   string " << cstring_ptr( &s) << std::endl;

  cstring_write(&s, "hello", sizeof("hello") );
  // cstring_push(&s, 'h' );
  // cstring_push(&s, 'e' );

  std::cout << "count " << cstring_count( &s) << "   string " << cstring_ptr( &s) << std::endl;


  std::cout << "last '" << ((char ) cstring_peek_last( &s) ) << "'" << std::endl;
  std::cout << "first " << ((char) cstring_peek_first( &s) )<< std::endl;


  
  // this isn't working???
  cstring_pop(&s );
  std::cout << "count " << cstring_count( &s) << "   string " << cstring_ptr( &s) << std::endl;

#endif


  std::cout << "push 'h'" << std::endl;
  cstring_push(&s, 'h' );
  assert( cstring_peek_last( &s) == 'h' ); 
  assert( cstring_peek_first( &s) == 'h' ); 
  
  std::cout << "last '" << ((char ) cstring_peek_last( &s) ) << "'" << std::endl;
  std::cout << "count " << cstring_count( &s) << "   string " << cstring_ptr( &s) << std::endl;

  std::cout << "pop 'h'" << ((char)cstring_pop(&s )) << std::endl;

  assert( cstring_peek_last( &s) == 'o' ); 

  std::cout << "last '" << ((char ) cstring_peek_last( &s) ) << "'" << std::endl;
  std::cout << "count " << cstring_count( &s) << "   string " << cstring_ptr( &s) << std::endl;


  for(unsigned i = 0; i < 5; ++i ) {
    cstring_pop(&s );
  }

  assert(cstring_count( &s) == 1);

  std::cout << "count " << cstring_count( &s) << "   string " << cstring_ptr( &s) << std::endl;

  // cstring_pop(&s );
}



#endif





