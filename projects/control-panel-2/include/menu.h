/*
  Rather than use arrays. think using a linked list. to support different size text. in same structure could be good.
  eg. allows text mapping the screen.
  could set up next pointers. for righ direction text flow .
*/

#ifndef MENU_H
#define MENU_H

#include "assert.h"
#include "curses.h"


// who owns the list...
// what structure do we use for the list.   an array?

/*
    Do we need to have concept of active controller.

    or only  draw if have element.
*/


struct ListController
{
  // eg.   apple in { bananas, apples, pears }
  // draw all elements in the list. (maybe except the active one).
  // we can pass the curses... no need to include here.
  int32_t rotary_begin;

  bool  focus;

  /*
    reading the index as a public variable ... versus injecting it.
    think that just reading it should be sufficient.

    likewise for focus
  
  */
  // int32_t & idx_;
  int32_t idx;

  void (*callback)(void *, unsigned );
  void *callback_ctx;



  ListController (/*int32_t & idx_ */) :
    rotary_begin(0),
    idx( 0),
    focus(false),
    callback( NULL),
    callback_ctx( NULL)
  { }

  void begin_edit(int32_t rotary);
  void finish_edit(int32_t rotary);
  void rotary_change(int32_t rotary);

  // set the number of items. and set the callback function.
  void set_callback( void (*)(void *, unsigned ), void *);

//   void commit_edit();
//  void event( int );
  //void draw( Curses &, bool active); // active indicates tells the controller  if it's active.  or just use begin_edit()?
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
  bool focus;

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

  double value_begin;

  bool focus;

  int32_t & idx;

  // TODO injected at the momment. need add()/remove() as well as unit and sign editing.
  double  & value;

  explicit DigitController(int32_t & idx_, double  & value_)
    : rotary_begin(0),
    focus(false),
    idx(idx_),
    value(value_),
    value_begin( 0 )
  { }

  /*
    rather than using aliases. instead just set_value( double value, etc).

  */

  void begin_edit(int32_t rotary);
  void finish_edit(int32_t rotary);
  void rotary_change(int32_t rotary);

  // don't think should be here.
  void draw(Curses &curses);

};








// we need an enum for which controller is active...


/*
  delegates and uses other controllers 
*/

struct MenuController
{
  // Should just know which controller is active { list, element, element }
  // probably should not take curses...
  // unless uses to take events


  Curses &curses;


  ListController    & list_controller;   // list_element controller
  ElementController & element_controller;   // enumerate
  DigitController   & digit_controller; // value controller

  unsigned active_controller; // 0 == list_controller

  explicit MenuController(Curses &curses_, ListController & list_controller_, ElementController & element_controller_, DigitController &digit_controller_)
    : curses(curses_),
    list_controller(list_controller_),
    element_controller( element_controller_),
    digit_controller( digit_controller_),

    active_controller(0)
    {
      // need to send an initial value

        // seems to lock/up. value not initialized yet.
        // list_controller.begin_edit( 0 );
    }


  //

  // void draw();
  void event( int);

};


/*
    OK. rather than using a callback.   we could inject the idx to use at construction time???

  It is dying on initialization.
*/

struct Menu
{
  //
  Curses & curses;
  MenuController & menu_controller;

  explicit Menu( Curses & curses, MenuController & menu_controller_ )
    : curses( curses ),
    menu_controller(menu_controller_)

  {
    printf("Menu controller constructor() %p\n", this );

    // want a list of strings
    // we don't have a container for these yet...
    char *keys[]      = { "whoot", "bar", "foo" }  ;
    double values[]   = { 123, 456, 789  }  ;

    // set callback
    menu_controller.list_controller.set_callback( trampoline, this);
  }

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

  void draw ();


};





#endif // MENU_H

