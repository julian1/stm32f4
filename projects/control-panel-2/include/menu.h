

#ifndef MENU_H
#define MENU_H

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
  // draw all items in the list. (maybe except the active one).
  // we can pass the curses... no need to include here.
  int32_t rotary_begin;


  ListController () :
    rotary_begin(0)
  { }

  void begin_edit(int32_t rotary);
  void rotary_change(int32_t rotary);


  void commit_edit();
  void event( int );
  void draw( Curses &, bool active); // active indicates tells the controller  if it's active.  or just use begin_edit()?
};



struct ItemController
{
  // maybe change name element controller
  //  eg. the  3 in 12345

  // we can pass the curses... no need to include here.
  int32_t rotary_begin;

  int32_t & idx;
  double & value;
  double & value_begin;

  // may want to pass this by reference. so can actually change the value.

  explicit ItemController(int32_t & idx_, double & value_, double & value_begin_)
    : rotary_begin(0),
    idx(idx_),
    value(value_),
    value_begin(value_begin_)
  { }



  void begin_edit(int32_t rotary);
  void rotary_change(int32_t rotary);

/*
  void add_element();   // { loc, key,val }

  void commit_edit();
  void event( int );
  void draw( Curses &, bool active);
*/
};


struct DigitController
{
  // change name ValueController. no because might have { sign, digits, unit }
  //  eg. which value of 123456789.
  // might also be used for unit mv,uV,V etc.

  // we can pass the curses... no need to include here.
  int32_t rotary_begin;

  int32_t & idx;

  // TODO injected at the momment. need add()/remove() as well as unit and sign editing.
  double  & value;
  double & value_begin;

  explicit DigitController(int32_t & idx_, double  & value_, double & value_begin_)
    : rotary_begin(0),
    idx(idx_),
    value(value_),
    value_begin(value_begin_)
  { }



  void begin_edit(int32_t rotary);
  void rotary_change(int32_t rotary);


  void draw(Curses &curses);

/*
  void add_element();   // { loc, key,val }

  void commit_edit();
  void event( int );
  void draw( Curses &, bool active);
*/
};








// we need an enum for which controller is active...


/*
  digitController
  unitController
*/

struct MenuController
{
  // Should just know which controller is active { list, item, element }
  // probably should not take curses...
  // unless uses to take events


  Curses &curses;


  ListController & list_controller;
  ItemController & item_controller;
  DigitController & digit_controller;

  int active_controller;

  explicit MenuController(Curses &curses_, ListController & list_controller_, ItemController & item_controller_, DigitController &digit_controller_)
    : curses(curses_),
    list_controller(list_controller_),
    item_controller( item_controller_),
    digit_controller( digit_controller_),

    active_controller(0)
    {  }


  //

  void draw();
  void event( int);

};

#endif // MENU_H

