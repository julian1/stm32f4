/*
  simple stl like container/adaptor for contiguous objects but using reserved memory buffer.
  simpler than std::vector<> with a custom allocator / calloc.

  emory is allocated up front. and can be stack or heap. no other reallocation.

  should add erase() etc. with interator...
  -----
  Perhaps be useful to have ring buffer following the same strategy
  -----------

  Also see. std::span<>   new in 2020.
    https://en.cppreference.com/w/cpp/container/span
*/

#include <stddef.h> // size_t
#include <assert.h> // assert


// IMPORTANT. the other advantage of this approach - is that pointers into the array won't be moved.

template<class T>
struct Vec
{
private:
  T *start, *end_;  // end_ is end of available memory. TODO rename.
  T *pos;

public:
  explicit Vec( T *start_, T *end__ )
    : pos(start_), start(start_), end_(end__)
  { }

  ////
  // const functions

  size_t size() const
  {
    return pos - start;
  }

  size_t empty() const
  {
    return pos == start;
  }

  size_t reserve() const
  {
    return end_ - start;
  }


  size_t max_size() const // reserve
  {
    return end_ - start;
  }

  ///////

  T *begin() const { return start; }
  T *end()  const  { return pos; }


  void push_back( const T &v )
  {
    assert(pos < end_);
    *pos++ = v;

    // c style. trampoline callback.
    // callback( callback_ctx, &v, enum add_item );
  }



  /////////////
  // non-const functions

  void clear()
  {
    // TODO should call destructors... no assignment...
    pos = start;
  }

  void pop_back()
  {
    assert(pos > start);
    --pos;
  }

  // erase, is expensive. maybe no need to include



};



#if 0


#include <algorithm>
#include <iostream>



int main()
{
  int ia[10];

  Vec<int>   x(ia, ia + 10);

  x.push_back(4);
  x.push_back(6);
  x.push_back(5);

  std::cout << "size " << x.size() << std::endl;
  std::cout << "max_size " << x.max_size() << std::endl;

  std::sort(x.begin(), x.end() );

  for(int *i = x.begin(); i != x.end(); ++i) {

    std::cout << *i  << std::endl;
  }

  int *j = std::min_element(x.begin(),  x.end() );

  std::cout << "min " << (j != x.end() ? *j : -123 ) << std::endl;
  std::cout << "empty " << x.empty()  << std::endl;



  x.pop_back();
  std::cout << "size " << x.size() << std::endl;

  return 0;
}
#endif



