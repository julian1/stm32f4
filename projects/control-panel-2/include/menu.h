
#pragma once

#include "assert.h"
#include "curses.h"
#include "usart.h"

struct DigitController;
struct ListController;


/*
  - done - fix the index ranges - so can have more/less than 3 items in drop-down.
  - done - change name drop_controller to page_controller.

  - need ability to inject a boolean option. for eg. autoranging on/off. true/false.


  - maybe change name Item  to  Page or PageItem or MenuItem.
*/


/*
  - OK. hang on. rather than using a variant. stl or our own. and making type explicit choice.
  - should we be using an opaque pointer.
  -------------

  ISSUE
  - issue - for non float/non int.   we don't need the digit control step.
  - eg. for bool, or enums. we just need to iterate the options.
*/




typedef void (*edit_t)( void *, unsigned idx, int delta );
typedef void (*copy_t)( const void *, void *dst, size_t sz );
typedef void (*format_t)( const void *, char *buf, size_t sz);
typedef  bool (*validate_t)( void *) ;

struct Value
{
  void *value;

  // actually separating the controller functions from the values - means less repetition. and filling things in.
  edit_t    edit;
  copy_t    copy;
  format_t  format ;
  validate_t validate ;

  // bool no_element_controller.

  Value(
    void *value_,

    edit_t edit_,
    copy_t copy_,
    format_t format_,
    validate_t validate_
  ) :
    value(value_),
    edit(edit_),
    copy(copy_),
    format(format_),
    validate(validate_)
  { }

};

// this will have to take the argument
// and modify it in place
void value_float_edit(double *x, int idx, int amount);

void value_float_copy( const double *x, void *dst, size_t sz );

void value_float_format( const double *x, char *buf, size_t sz);

void value_float_format2( const double *x, char *buf, size_t sz);




struct Item
{
  // this is Items plural. or MenuItem single.

  char    *name;
  char    **keys;
  // double  *values ;
  Value   *values ;
  unsigned  n;

  // may also want no list controller. not sure.
  bool    no_element_controller;

  Item(
    char    *name,
    char    **keys,
    Value   *values ,
    unsigned  n
 ) :
    name(name),
    keys(keys),
    values(values),
    n(n),
    no_element_controller(false)
  { }


};



struct PageController
{
  // maybe rename MenuController.
  // and rename the MenuController to DelegatingController

  ListController & list_controller;

  // eg.   apple in { bananas, apples, pears }
  // draw all elements in the list. (maybe except the active one).
  // we can pass the curses... no need to include here.
  int32_t rotary_begin;
  bool    focus;
  int32_t idx;


  // OK. how do we handle this data?
  // perhaps a switch table would be easier.

  Item **items;
  unsigned n;

  PageController (  ListController & list_controller )
    :
    list_controller( list_controller),
    rotary_begin(0),
    focus(false),
    idx( 0),

    items(items),
    n(n)
  {
  }

    // will propagate down.
    // actually should probably not do in constructor.
    // list_controller.set_value( items[0 ] );

  void set_value(Item **items, unsigned n );


  void begin_edit(int32_t rotary);
  void finish_edit(int32_t rotary);
  void rotary_change(int32_t rotary);

};









struct ListController
{

  DigitController & digit_controller;

  // eg.   apple in { bananas, apples, pears }
  // draw all elements in the list. (maybe except the active one).
  // we can pass the curses... no need to include here.
  int32_t rotary_begin;
  bool    focus;
  int32_t idx;

  Item  *item ;

  ListController (  DigitController & digit_controller /*, Item *item */ )
    :
    digit_controller( digit_controller ),
    rotary_begin(0),
    focus(false),
    idx( 0),
    item( NULL )
  { }

  // should the value be a pointer ????
  // so that it manipulates the real value.
  // called by PageController
  void set_value( Item *item );


  void begin_edit(int32_t rotary);
  void finish_edit(int32_t rotary);
  void rotary_change(int32_t rotary);

  // set the number of items. and set the callback function.
  // void set_callback( void (*)(void *, unsigned ), void *);

};


/*
  The reason we use use alias for value. is that we inject the same value into two controllers.

*/

struct ElementController
{
  // element of a double number
  //  eg. the  3 in 12345

  // we can pass the curses... no need to include here.
  int32_t rotary_begin;
  bool    focus;
  int32_t & idx;

  /*
    idx is by reference because it is shared state with digit controller.
  */

  explicit ElementController(int32_t & idx_ )
    : rotary_begin(0),
    focus(0),
    idx(idx_)
  { }


  // should not pass the rotary... ?
  void begin_edit(int32_t rotary);
  void finish_edit(int32_t rotary);
  void rotary_change(int32_t rotary);

};


struct DigitController
{
  // change name ValueController. no because might have { sign, digits, unit }
  //  eg. which value of 123456789.
  // might also be used for unit mv,uV,V etc.

  // we can pass the curses... no need to include here.
  int32_t rotary_begin;
  bool focus;

  // idx is shared state with element controller.
  // could alternatively - use set_pos(), but it works well enoughvalue
  int32_t & idx;

  // which value is being edited to be updated.
  Value *value;

  // Ughgh... we cannot copy an opaque (void *) value...
  // to get the starting value.
  // double value_begin;

  char value_begin[100] ; // large enough to hold the largest value

  explicit DigitController(int32_t & idx_)
    : rotary_begin(0),
    focus(false),
    idx(idx_),
    value( NULL ) // ,
    // value_begin( 0 )
  { }

  /*
    rather than using aliases. instead just set_value( double value, etc).

  */

  // should the value be a pointer ????
  // so that it manipulates the real value.
  // void set_value( double * value );
  void set_value( Value * value );

  void begin_edit(int32_t rotary);
  void finish_edit(int32_t rotary);
  void rotary_change(int32_t rotary);

  // don't think should be here.
  void draw(Curses &curses);

};









struct MenuController
{
  /*
    - DelegatingController.
    - just takes events - and delegates and uses other controllers
    - no responsibility for drawing or anything else

  */

  PageController       & page_controller;
  ListController    & list_controller;
  ElementController & element_controller;
  DigitController   & digit_controller;

  unsigned active_controller;

  explicit MenuController(PageController & page_controller_, ListController & list_controller_, ElementController & element_controller_, DigitController &digit_controller_)
    :
    page_controller(page_controller_),
    list_controller(list_controller_),
    element_controller( element_controller_),
    digit_controller( digit_controller_),

    active_controller(0)
    {
        // usart data structures are up now
        // set the list controller as active

        //
        // list_controller.begin_edit( 0 );
        page_controller.begin_edit( 0 );
    }

  void event( int);

};



struct Menu
{
  /*
    responsible for drawing. so needs other controllers. to know which is active etc.
    should potentiallly rename to menu view:w.
    - maybe rename MenuDrawer etc.

    - could probably be turned into a single function.
  */

  PageController       & page_controller;
  ListController    & list_controller;
  ElementController & element_controller;
  DigitController   & digit_controller;



  explicit Menu( PageController       & page_controller_, ListController & list_controller_, ElementController & element_controller_, DigitController &digit_controller_)
    :
    page_controller(page_controller_),
    list_controller(list_controller_),
    element_controller( element_controller_),
    digit_controller( digit_controller_)
  {
    printf("Menu controller constructor() %p\n", this );

  }

/*
  static void trampoline( void *object, unsigned idx)
  {
    // static trampoline function. simpler than using boost::function

    printf("trampoline\n");
    assert(object);
    ((Menu *)object)-> focus_changed(idx);
  }


  void focus_changed( unsigned idx)
  {
    printf("Menu controller focus_changed() %p\n", this );
  }
*/


  // void draw ();

  void draw( Curses & curses  );


};





