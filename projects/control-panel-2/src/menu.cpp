
#include <libopencm3/stm32/timer.h>

#include "menu.h"
#include "ui_events.h"
#include "assert.h"

#include "streams.h"








//////////////////

void ListController::begin_edit(int32_t rotary)
{
  usart_printf("list controller begin_edit()\n");
  rotary_begin = rotary;
}


void ListController::rotary_change(int32_t rotary)
{
  usart_printf("list controller rotary_change()  %d\n", (rotary - this->rotary_begin) / 4 % 10  );

  ;

}

//////////////////
// TODO change name ElementController.   eg.  5 in +3456mV.

 void ItemController::begin_edit(int32_t rotary)
{

  rotary_begin = rotary;

  // copy the value
  value_begin = value;

  usart_printf("item controller begin_edit() - copying value_begin %f \n", value_begin);
}



void ItemController::rotary_change(int32_t rotary)
{
  // actually. don't think we require any change
  // delta is the value
  // We may want to inject this idx in. so it can be shared with the digit controller.

  // OK. we need to sign extend

  // this->idx = (int16_t(rotary) - this->rotary_begin) / 4 ;
  this->idx = ( this->rotary_begin - int16_t(rotary)  ) / 4 ;

  usart_printf("item controller rotary_change() idx  %d\n", this->idx );

  // EXTR. I think we might pass the digit as digit index
}







///////////////////

void DigitController::begin_edit(int32_t rotary)
{
  usart_printf("digit controller begin_edit()\n");
  rotary_begin = rotary;

}


static double edit_float_value(double x, int idx, int amount)
{
  // must be float for negative idx
  // some math.h have pow10(double)
  double u = pow(10, idx);
  double delta = amount * u;
  // printf("idx=%d amount=%d u=%f \n", idx, amount, u );
  return x + delta;
}



/*
todo.
// need highlight the digit being editing and highlighting the position on screen.
// thne maybe try to add sign/unit.

  transition
    - from block/invert highlight for item.
    - to blinking to digit edit.
*/

void DigitController::rotary_change(int32_t rotary)
{
  /*
    - may be better to print the value into an array. then modify.
  */
  // IMPORTANT - value is allowed to go greater/lesser than 0-10.
  // eg. wind on voltage from 9V to 12V.

  int32_t delta = (int16_t(rotary) - this->rotary_begin) / 4 ;
  // int32_t delta = (rotary - this->rotary_begin) / 4 /*% 10 */;

  // we are adding delta each time.
  // while we want to adjust the value.
  // so think we need to record the starting value.
  // OR. we don't change value.

  // this->value = this->value_begin + edit_float_value(this->value , this->idx, delta );

  // when we change the digit we edit - then we have to adjust

  this->value =  edit_float_value(this->value_begin  , this->idx, delta );

  usart_printf("digit controller rotary_change()  idx=%d delta=%d  value=%f\n", this->idx, delta, this->value );


}

/*
  // shifting the value right depending how many digits are on the rhs of
  decimal point. while editing is ok. in order that the rhs fields of list align.
*/

static size_t dot_position( char *s )
{
  char *p = s;
  while(*p && *p != '.')
    ++p;

  return p - s;
}


static char * format_float(char *s, size_t sz, int suffix_digits, double value)
{
  /*
    %f. always adds '.' and suffix '0' even input is rounded.
    this will prefix with '-'
  */
  // format
  size_t n = snprintf(s, sz, "%.*f", suffix_digits, value);
  return s;
}










void DigitController::draw(Curses &curses)
{

  // should call the digit controller. which can draw the value
  char buf[100];

  // grid spacing for text is quite deltaerent than for keypad button spacing.
  color_pair_idx(curses, 0); // blue/white
  //effect(a, 0x01);        // invert
  effect(curses, 0x00);        // normal
  font(curses, &arial_span_18 ); // large font
  to(curses, 0, 4);
  // snprintf(buf, 100, "%f ", this->value);  // our edited value... NOTE. needs extra of active digit etc..

  format_float(buf, 100, 6, this->value);
  text(curses,  buf , 1);

  // now we want to place an effect on the active digit.
  // OK. position has to be where the decimal dot is.  and then negative
  // want a function to give us the offset of the '0' character.
  // and the idx bounds also.    perhaps just print into a buffer? and return all of this.
  // no. better to specify prefix/postfix digit count.
  // but also have issue of prefix sign.


  size_t dot_x = dot_position( buf );

  to(curses, 0 + dot_x - this->idx, 4);
  effect( curses, 0x01 ); // invert.... this just sets the mode.
  // effect( curses, 0x10 ); // blink. doesn't seem to work...
  // set the effect at current position
  ch_effect( curses);
  // text(curses,  "x", 1);



  effect(curses, 0x00);        // normal
  to(curses, 1, 5);
  snprintf(buf, 100, "%ld   ", (int32_t) timer_get_counter(TIM1));
  text(curses, buf , 1);
  // text(curses, "3.4mCurses", 1);




  color_pair_idx(curses, 1); // blue/white
  to(curses, 0, 10);
  text(curses,  "whoot", 1);
}




#if 0
static void draw_test4(Curses &curses)
{
  // should call the digit controller. which can draw the value

  // grid spacing for text is quite deltaerent than for keypad button spacing.
  color_pair_idx(curses, 0); // blue/white
  //effect(a, 0x01);        // invert
  effect(curses, 0x00);        // normal
  font(curses, &arial_span_18 ); // large font
  to(curses, 0, 4);
  text(curses, "+99.456mV", 1);


  char buf[100];
  snprintf(buf, 100, "%ld   ", (int32_t) timer_get_counter(TIM1));
  effect(curses, 0x00);        // normal
  to(curses, 1, 5);
  text(curses, buf , 1);
  // text(curses, "3.4mCurses", 1);
}



static void draw(Curses &a )
{
   // return 0;
}
#endif





void MenuController::event(int event_)
{

  // uint32_t to int32_t.
  // actually if using modulus it shouldn't matter
  int32_t rotary = timer_get_counter(TIM1);

  if(event_ == ui_events_button_right ) {
    // switch the controller

    ++ this->active_controller;
    if( this->active_controller > 2)
      this->active_controller = 2;

    // we also want to call being() on the active controller
    // could use virtual functions and then index. if want.

    // actually we should perhaps do a exit_edi() from what we are coming from.

    switch( this->active_controller ) {
      case 0:  list_controller.begin_edit(rotary);  break;
      case 1:  item_controller.begin_edit(rotary); break;
      case 2:  digit_controller.begin_edit(rotary); break;
    }
  }

  else if(event_ == ui_events_button_left ) {

    -- this->active_controller;

    if( this->active_controller < 0 )
      this->active_controller = 0;

    // i think we have to pass the rotary...
    // when we go navigate backwards... is this a finish_edit() ?

  }

  else if (event_ == ui_events_rotary_change ) {



    // rotary event should pipe through to the active controller. OR. get the timer and pass it....
    // using begin( rotary )
    // pass the event directly ???


    switch( this->active_controller ) {
      case 0:  list_controller.rotary_change(rotary);  break;
      case 1:  item_controller.rotary_change(rotary); break;
      case 2:  digit_controller.rotary_change(rotary); break;
    }

  }

  // usart_printf("menu controller event %ld\n", event_);
  // usart_printf("new active controller %ld\n", this->active_controller );
}




void MenuController::draw()
{
  // usart_printf("menu controller test() \n");

  // draw( this->curses );

  // persist the page that we need to draw
  static int page = 0; // page to use
  page = ! page;

  // set up our buffer
  pixfmt_t  pixf(  page *  272 );
  rb_t    rb(pixf);

  // rb.clear(agg::rgba(1,1,1));     // bg white .
  rb.clear(agg::rgba(0,0,0));       // bg black

  // uint32_t start = system_millis;
  // ok, this works. so maybe there is memory corruption somewhere.

  ////////////////////////////////////

  // draw_test4( curses );
  digit_controller.draw(curses);


  int blink = (system_millis / 500) % 2;
  usart_printf("blink %u\n", blink );

  render( curses , rb,  blink );

  // lcd synchronization, wait until not in vertical blanking mode
  while( getTear() ) {
    // usart_printf("tear hi\n" );
  };

  // flip the newly drawn page in
  setScrollStart( page *  272 );




}

#if 0
// so we want a double.

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





static int edit_int_value(int x, int idx, int amount)
{
  /*
   printf("done %d\n", edit_int_value( 999999, 3, -1) ); =>
    done 999899

  printf("done %d\n", edit_int_value( 999999, 10, 1) );
    done 1000999999

  same approach can probably work for floats.
  note. this works for adding extra high digits. eg. from 10 to 1000.
  */
  int u = int_pow(10, idx);
  int hi = x / u * u;
  int lo = x - hi ;
  // printf("lo %d\n", lo );
  assert(hi + lo == x);

  int u2 = int_pow(10, idx - 1);

  int j = (lo + amount * u2 ) % u ;
  return hi + j;
}
#endif
