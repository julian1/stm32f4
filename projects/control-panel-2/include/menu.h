/*
  Rather than use arrays. think using a linked list. to support different size text. in same structure could be good.
  eg. allows text mapping the screen.
  could set up next pointers. for righ direction text flow .
*/

#ifndef MENU_H
#define MENU_H

#include "assert.h"
#include "curses.h"
#include "usart2.h"

struct DigitController;


struct ListController
{

  DigitController & digit_controller;

  // eg.   apple in { bananas, apples, pears }
  // draw all elements in the list. (maybe except the active one).
  // we can pass the curses... no need to include here.
  int32_t rotary_begin;
  bool    focus;
  int32_t idx;


  char **keys;
  double *values;

  ListController (  DigitController & digit_controller, char **keys, double *values, size_t n  )
    :
    digit_controller( digit_controller ),
    rotary_begin(0),
    focus(false),
    idx( 0),
    // callback( NULL),
    // callback_ctx( NULL),

    keys( keys ),
    values( values)
  { }

  void begin_edit(int32_t rotary);
  void finish_edit(int32_t rotary);
  void rotary_change(int32_t rotary);

  // set the number of items. and set the callback function.
  // void set_callback( void (*)(void *, unsigned ), void *);


  // void draw(Curses &curses);

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
  double  *value;
  double value_begin;

  explicit DigitController(int32_t & idx_)
    : rotary_begin(0),
    focus(false),
    idx(idx_),
    value( NULL ),
    value_begin( 0 )
  { }

  /*
    rather than using aliases. instead just set_value( double value, etc).

  */

  // should the value be a pointer ????
  // so that it manipulates the real value.
  void set_value( double * value );

  void begin_edit(int32_t rotary);
  void finish_edit(int32_t rotary);
  void rotary_change(int32_t rotary);

  // don't think should be here.
  void draw(Curses &curses);

};








// we need an enum for which controller is active...


/*
*/

struct MenuController
{
  /*
    just takes events - and delegates and uses other controllers
    no responsibility for drawing

  */

  ListController    & list_controller;
  ElementController & element_controller;
  DigitController   & digit_controller;

  unsigned active_controller;

  explicit MenuController(ListController & list_controller_, ElementController & element_controller_, DigitController &digit_controller_)
    :
    list_controller(list_controller_),
    element_controller( element_controller_),
    digit_controller( digit_controller_),

    active_controller(0)
    {
      /*
        problem ... all this constructor stuff is happening before the usart is up and running.

      */

//        usart_flush();
 //       printf("*********\n");
        // set active element
        list_controller.begin_edit( 0 );
    }

  void event( int);

};


/*
    OK. rather than using a callback.   we could inject the idx to use at construction time???

  It is dying on initialization.
*/

struct Menu
{
  /* 
    responsible for drawing. so needs other controllers. to know which is active etc.
    should potentiallly rename to menu view:w.

  */
  Curses & curses;
  // MenuController & menu_controller;

  ListController    & list_controller;
  ElementController & element_controller;
  DigitController   & digit_controller;



  explicit Menu(Curses & curses, ListController & list_controller_, ElementController & element_controller_, DigitController &digit_controller_)
  // explicit Menu( Curses & curses, MenuController & menu_controller_ )
    : curses( curses ),
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

  void draw ();
  void draw1 ( Curses & );


};





#endif // MENU_H

