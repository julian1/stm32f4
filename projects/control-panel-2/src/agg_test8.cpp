

#include <libopencm3/stm32/timer.h>

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


#include "curses.h"

/*
  ok. - so how to treat events - need to route.  directly  or queue.
      - we cannot get the timer value in the interupt.
      - also provides a queue for update.
    ------------
    inject a mode enum. for which selector/controller is active.



  -------------
  controller for position and draw.
  controller for edit.
  --
  have a queue for events.   button press, rotary ?  can then just process in the main loop.

*/

struct IntegerSelector;
struct UnitSelector;

struct ListSelector
{
  /*
  // eg list of menu items.
  // when one is active to edit we will stuff it into the integer selector (to edit individual digits.
    can scroll them / focus.
    when one focused and hit right button - then we put item into the individual digit selector/unit selector as needed.unit se
  */

  IntegerSelector   & integer_selector;
  UnitSelector      & unit_selector;

  // name, value pairs.
  // std::vector<



};



// polymorphism via virtual functions or just a regular enum argumented function
// actually we do need the buf - for the exploded view. or at

struct IntegerSelector
{
  Curses &a;

  // this is the exploded view of the value
  // Integer selector. need a different one for double. with a dot'
  int & value;
  int edit_value; // value being edited. precomitted_value;
                      // if we are going to edit multiple values...

  // unsigned x, y;  position.
  unsigned select_idx;

  unsigned rotary_on_begin;  // rotary value at beginning of edit

  explicit IntegerSelector(Curses &a)
    : a(a),
    value(value),
    edit_value(0),
    select_idx(0),
    rotary_on_begin(0)
    {  }

  // when item becomes active.
  void set_value( int value_ , int x, int y ) { value = value_; }

  void begin_edit(  ) 
  {  
    rotary_on_begin = timer_get_counter(TIM1);
  }

  void event( int ch)
  {
    switch(ch) { 



    };

  }

};


// treat non-virtual functions as functions.

void draw ( IntegerSelector & s)
{
  /*
    if we are not editing it. then don't draw it.
    let the item list view draw the value.
  maybe

  */
  // not polymorphic.  that's ok.

  char buf[ 100 ] ;
  snprintf(buf, 100, "%d", s.value);
  unsigned len = strlen(buf);

  for(unsigned i = 0; i < len; ++i) {

    to(s.a, 5 + i, 6);

    // dial focus.
    if( i == s. select_idx)
      effect(s.a, 0x01) ;
    else
      effect(s.a, 0x00);

    // output the character
    ch_text( s.a, buf[ i] );
  }
}

// begin_edit/ end edit ?


#if 0
// https://stackoverflow.com/questions/29787310/does-pow-work-for-int-data-type-in-c

static int int_pow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp % 2)
           result *= base;
        exp /= 2;
        base *= base;
    }
    return result;
}

#endif


void rotary_event( IntegerSelector & s )
{

  // take the rotary value ...
  //    we don't need to use 'a' and adding .... just rotate

  int rotary = timer_get_counter(TIM1);

  // s.value +=  ( int_pow( 10, s.select_idx )  *   rotary )   ;  // we want to change the decimal value...

  // s.value =


}


/*
  - actually can use a strategy pattern.
  - eg. just use an interface and swap the strategy.

*/
// think we just pass  events down. 

static void draw_test2(Curses &a )
{
  // grid spacing for text is quite different than for keypad button spacing.
  color_pair_idx(a, 0); // blue/white
  //effect(a, 0x01);        // invert
  effect(a, 0x00);        // normal
  font(a, &arial_span_18 ); // large font
  to(a, 0, 4);
  text(a, "+23.456mV");

  char buf[100];
  snprintf(buf, 100, "%d   ", timer_get_counter(TIM1));

  effect(a, 0x00);        // normal
  to(a, 1, 5);
  text(a, buf );
  // text(a, "3.4mCurses");
}



extern int agg_test8(  Curses &a )
{

  // static Curses a( 33, 17, 14, 16 );

#if 0
  static bool first = true;
  if(first) {
    // move these to constructor?

    // trying to init both of these hangs...

    usart_printf("sizeof(agg::rgba) %u\n", sizeof(agg::rgba));
    usart_printf("sizeof(Curses) %u\n", sizeof(Curses));

    // should not be initializing here, like this
    init( a);
                   // but
    first = false;
  }
#endif

  // persist the page that we need to draw
  static int page = 0; // page to use
  page = ! page;

  // set up our buffer
  pixfmt_t  pixf(  page *  272 );
  rb_t    rb(pixf);

  // rb.clear(agg::rgba(1,1,1));     // bg white .
  rb.clear(agg::rgba(0,0,0));       // bg black

  uint32_t start = system_millis;
  // ok, this works. so maybe there is memory corruption somewhere.

  ////////////////////////////////////

  draw_test2( a );


  int blink = (system_millis / 500) % 2;
  // usart_printf("blink %u\n", blink );

  render( a , rb,  blink );

  // lcd synchronization, wait until not in vertical blanking mode
  while( getTear() ) {
    // usart_printf("tear hi\n" );
  };



  // flip the newly drawn page in
  setScrollStart( page *  272 );


    return 0;
}







// we need the interupts as events . so we can change the state.


/*
  make controllers for,
    unit selector
    number selector
    list selector

    then inject some test values in.

    then events - need to be controlled - to shift between them . just need to coordinate which one is active.
    use a state machien to switch between them
    ---

  these are wanted - for when we are not using a standard editing layout.

*/

#if 0
struct X
{
  // get the values to display from the tree.
  // but control how we display
  // Tree * parent

  unsigned child_idx;

  bool child_is_exploded;
  unsigned exploded_idx;

  Curses &a;


  // draw the children.

};
#endif





/*
  think we have a single one of these.

  - game programming controller - manage a list of items.
      - eg. we don't want to be creating controllers.

  - when the list item becomes active.   it's going to have to insert the elements into the value controller editor.

  - we cannot


  -----------------
  begin_edit
    copy the rotary delta.
    select which digit we are going to edit.
    copy value to edit value .

  cancel_edit

  commit_edit
    copy edited val to real value.
    this is going to have to apply to ( unit & value ) .   perhaps the entire page. with separate cancel /  OK. buttons
  -----------------

*/

struct VRangeSelector
{


};





