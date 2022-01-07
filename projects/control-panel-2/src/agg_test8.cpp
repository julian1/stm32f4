

#include "curses.h"

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

  A &a;


  // draw the children.

};
#endif



// polymorphism via virtual functions or just a regular enum argumented function
// actually we do need the buf - for the exploded view. or at  

struct IntegerSelector
{
  A &a;

  // this is the exploded view of the value
  // Integer selector. need a different one for double. with a dot'
  int & value;
  int edit_value; // value being edited. precomitted_value;

  // unsigned x, y;  position.
  unsigned select_idx;

  explicit IntegerSelector(A &a, int &value)
    : a(a),
    value(value),
    edit_value(0),
    select_idx(0)
    {  }
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
    textch( s.a, buf[ i] );
  }
}

// begin_edit/ end edit ?

void rotary_event( IntegerSelector & s )
{



}


/*
  begin_edit
    copy the rotary delta.
    select which digit we are going to edit.
    copy value to edit value .

  cancel_edit
    
  commit_edit
    copy edited val to real value. 
    this is going to have to apply to ( unit & value ) .   perhaps the entire page. with separate cancel /  OK. buttons

*/

struct VRangeSelector
{


};




static void draw_test2(A &a )
{
  // grid spacing for text is quite different than for keypad button spacing.
  color_pair_idx(a, 0); // blue/white
  //effect(a, 0x01);        // invert
  effect(a, 0x00);        // normal
  font(a, &arial_span_18 ); // large font
  to(a, 0, 4);
  text(a, "+23.456mV", 1);

  effect(a, 0x00);        // normal
  to(a, 1, 5);
  text(a, "3.4mA", 1);


}



extern "C" int agg_test8()
{

  static A a( 33, 17, 14, 16 );

  static bool first = true;
  if(first) {
    // move these to constructor?

    // trying to init both of these hangs...

    usart_printf("sizeof(agg::rgba) %u\n", sizeof(agg::rgba));
    usart_printf("sizeof(A) %u\n", sizeof(A));

    // should not be initializing here, like this
    init( a);
                   // but
    first = false;
  }

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



