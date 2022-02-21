
#include <libopencm3/stm32/timer.h>

#include "menu.h"
#include "ui_events.h"
#include "assert.h"

#include "streams.h"


/*

    - use centre button to drill in. and other button to drill out.
    - controller exposes idx and focus as external vars. to suggest the active items.

    -----
    list controller - passes item to edit to the element controller.
                    - probably want a structure for this. eg. bounds.


*/


/*
  validation function - eg. bounds should be passed as a function.
  probably also the formatting function.
    eg. for number of digits. rather than specify as a value.
*/


static char * format_float(char *s, size_t sz, int suffix_digits, double value)
{
  /*
    %f. always adds '.' and suffix '0' even input is rounded.
    this will prefix with '-'
  */
  // format
  size_t n = snprintf(s, sz, "%.*f", suffix_digits, value);
  UNUSED(n);
  return s;
}




/*
  // shifting the value right depending how many digits are on the rhs of
  decimal point. while editing is ok. in order that the rhs fields of list align.
*/

static size_t dot_position( char *s )
{
  // given a string, return the dot or eos.
  char *p = s;
  while(*p && *p != '.')
    ++p;

  return p - s;
}





void value_float_edit(double *x, int idx, int amount)
{
  /*
    this isn't working with the decimal point
  */

  printf("value_float_edit x=%f   idx=%d amount=%d \n", *x, idx, amount );

  // skip decimal point. should perhaps be done outside here.
  // index
  if (idx > 0 ) {
    --idx;
  }

  // must be float for negative idx
  // some math.h have pow10(double)
  double u = pow(10, idx);
  double delta = amount * u;
  // printf("idx=%d amount=%d u=%f \n", idx, amount, u );

  *x = *x + delta;

  // return x + delta;
}


void value_float_copy( const double *x, void *dst, size_t sz )
{
  /*
    UGGH. no we want to be copying all the functions as well.
    No. just the underlying void object.
  */
  assert( sizeof(double) <= sz);

  // void *memcpy(void *dest, const void *src, size_t n);
  memcpy(dst, x, sizeof(double) );
}



void value_float_format( const double *x, char *buf, size_t sz)
{

    char buf2[100];
    // char buf[100];

    snprintf(buf, sz,  "%smV" , format_float(buf2, 100, 6, *x  ));
}



//////////

// Dropdown. controller.
// PageController



void PageController::set_value(Item **items_, unsigned n_ )
{
  usart_printf("page_controller - set_value()\n");
  assert(items_);

  items = items_;
  n = n_;

  //
  idx = 0;

  // propagate/push down
  list_controller.set_value( items[0] );
}


void PageController::begin_edit(int32_t rotary)
{

  usart_printf("page_controller - rotary_begin set to %d\n", rotary_begin);

  rotary_begin = rotary - idx;    // remember position from last time. works.
  focus = true;
}


void PageController::finish_edit(int32_t rotary)
{
  focus = false;
}

void PageController::rotary_change(int32_t rotary)
{

  usart_printf("page_controller - rotary_change\n");

  this->idx = rotary - this->rotary_begin;


  // bounds
  if(idx < 0) {
    rotary_begin = rotary;
    this->idx = rotary - this->rotary_begin;
    assert( this->idx == 0);
    return ; // nothing to do
  }
  else if(idx >= n ) {
    rotary_begin = rotary -  ( n - 1 );
    this->idx = (rotary - this->rotary_begin);
    assert( this->idx == n - 1);
    return;
    // idx = n - 1;
  }

  assert(items);
  // I think we need to set on construction also.
  // but it is reverse order.
  list_controller.set_value( items[ idx ]   );
}






//////////////////////////////

/*
  list controller
    - only really needs to keep track of the size/number of elements in the model. to iterate them
    - but also needs to push the active item into the element controller.
        but does *not* need to know about the keys.

    - thing to edit. is going to be complicated. eg. bounds. and display resolution.
*/


void ListController::set_value( Item *item_ )
{
  usart_printf("list_controller - set_value()\n");


/*
  printf("list_controller this %p\n", this);

  printf("keys[0] before %s\n", keys[0] );
  keys = keys_;

  printf("keys[0] now %s\n", keys[0] );

  values = values_;
  n = n_;
*/
  assert(item_);
  item = item_;

  digit_controller.set_value( & item->values[ 0]   );  // should use the idx
}


void ListController::begin_edit(int32_t rotary)
{
  usart_printf("list controller begin_edit()  idx=%d rotary=%d\n", idx, rotary);

  rotary_begin = rotary - idx;    // remember position
                                  // from last time. works.

  usart_printf("rotary_begin set to %d\n", rotary_begin);

  focus = true;
}


void ListController::finish_edit(int32_t rotary)
{
  usart_printf("list controller finish_edit()\n");

  focus = false;
}



void ListController::rotary_change(int32_t rotary)
{
  // calc idx
  this->idx = rotary - this->rotary_begin;

  assert(item );
  unsigned n = item->n;

  // bounds
  if(idx < 0 )  {
    rotary_begin = rotary;
    // recalculate
    this->idx = rotary - this->rotary_begin;
    assert( this->idx == 0);
    return;
  }
  else if( idx >= n) {
    rotary_begin = rotary - ( n - 1 );
    this->idx = (rotary - this->rotary_begin);
    assert( this->idx == n - 1);
    return;
  }

  usart_printf("list controller rotary_change()  idx = %d\n", idx    );

  // set the active value
  digit_controller.set_value ( & item->values[ idx ] );

}


/*
  - I think injecting and editing a reference to the value might be more interesting.
    than copying.

    - it will have additional properties. like number of digits. smallest unit etc.
    - whether to commit a value.  can be done separately, as a separate concern.
  --------------------

*/




//////////////////
//////////////////


void ElementController::begin_edit(int32_t rotary)
{
  // copy the value
  // value_begin = value;
  focus = 1;

  // usart_printf("element controller begin_edit() - value_begin=%f\n", value_begin);
  usart_printf("element controller begin_edit()\n");

  // OK. this works. when return to an edit.
  rotary_begin = idx + rotary;

  usart_printf("rotary_begin %d\n", rotary_begin );
  usart_printf("idx          %d\n", idx );
}



void ElementController::finish_edit(int32_t rotary)
{
  usart_printf("element controller finish_edit() \n");
  focus = 0;
}



void ElementController::rotary_change(int32_t rotary)
{
  usart_printf("element controller rotary_change() rotary %d\n", rotary );

  this->idx = this->rotary_begin - rotary ;   // negative, because dot position is negative

  usart_printf("idx  %d\n", this->idx );

}







///////////////////

/*
  requires value_begin, and rotary_begin  - to edit the value.

*/
void DigitController::begin_edit(int32_t rotary)
{
  usart_printf("digit controller begin_edit()\n");
  rotary_begin = rotary;
  focus = true;

  // set_value should have been called before here...

  // OK. this thing with set_value is hard.
  // because of when to call it.

  assert(this->value );

  // this->value_begin = * this->value ;

  // we don't have the copy

  // value_begin is *not* a Value. it is a Value.value

  // OK.
  // problem is that its

  value->copy(  value->value,   value_begin, 100 );

}


void DigitController::finish_edit(int32_t rotary)
{
  usart_printf("digit controller finish_edit()\n");

  focus = false;
}



void DigitController::set_value( Value * value_ )
// void DigitController::set_value( double * value_ )
{
  usart_printf("digit_controller - set_value()\n");

  assert(value_);
  value = value_;
}



/*
todo.
  transition
    - from block/invert highlight for element.
    - to blinking to digit edit.
*/

void DigitController::rotary_change(int32_t rotary)
{
  /*
    - may be better to print the value into an array. then modify.
  */
  // IMPORTANT - value is allowed to go greater/lesser than 0-10.
  // eg. wind on voltage from 9V to 12V.

  int32_t delta = rotary - this->rotary_begin;


  usart_printf("digit controller rotary_change()  idx=%d delta=%d  value=%f\n", this->idx, delta, this->value );

  // assert( this->value  );
  // * this->value = value_float_edit(this->value_begin, this->idx, delta );

/*
  double tmp = this->value_begin;
  value_float_edit( &tmp, this->idx, delta );
  *this->value = tmp;
*/
  char buf[100]; // temporary.

  // void (*copy)( const void *src, void *dst, size_t sz );

  // this is WRONG. value->begin is not a full value...
  // value->copy(  this->value_begin, buf, 100 );

  memcpy( buf, this->value_begin, 100); // assume enough. and assume pod.

  // void (*edit)( void *, unsigned idx, int delta ) ;
  value->edit(  buf ,  this->idx, delta );

  value->copy(  buf , value->value , 100 );
}








void MenuController::event(int event_)
{

  // uint32_t to int32_t.
  // actually if using modulus it shouldn't matter
  int32_t rotary__ = timer_get_counter(TIM1);

    // rotary event should pipe through to the active controller. OR. get the timer and pass it....
    // force sign extention.
  int32_t rotary = int16_t(rotary__)  >> 2    ;


  printf("* converted rotary %ld\n", rotary );



  if(event_ == ui_events_button_right
    || event_ == ui_events_button_left )
  {

    // candidate new controller
    int cand = this->active_controller;

    if(event_ == ui_events_button_right ) {
      // don't move in futhre or generate events.
      if( this->active_controller >= 3)  {
        printf("at limit\n");
        this->active_controller = 3;
        return;
      }
      printf("move in\n");
      ++cand;
    }
    else if(event_ == ui_events_button_left ) {
      if( this->active_controller <= 0 ) {
        this->active_controller = 0;
        printf("at limit\n");
        return;
      }
      printf("back out\n");
      -- cand;
    }

    // now generate events
    if(cand != this->active_controller) {

      switch( this->active_controller) {
        case 0:  page_controller.finish_edit(rotary);  break;
        case 1:  list_controller.finish_edit(rotary);  break;
        case 2:  element_controller.finish_edit(rotary); break;
        case 3:  digit_controller.finish_edit(rotary); break;
      }

      this->active_controller = cand;

      switch( this->active_controller ) {
        case 0:  page_controller.begin_edit(rotary);  break;
        case 1:  list_controller.begin_edit(rotary);  break;
        case 2:  element_controller.begin_edit(rotary); break;
        case 3:  digit_controller.begin_edit(rotary); break;
      }

    }

  }


  else if (event_ == ui_events_rotary_change ) {

    switch( this->active_controller ) {
      case 0:  page_controller.rotary_change(rotary);  break;
      case 1:  list_controller.rotary_change(rotary);  break;
      case 2:  element_controller.rotary_change(rotary); break;
      case 3:  digit_controller.rotary_change(rotary); break;
    }

  }

  // usart_printf("menu controller event %ld\n", event_);
  // usart_printf("new active controller %ld\n", this->active_controller );
}










/*
  keithley 7.5 dmm7510.
    has enter and exit buttons and rotary and that's all.
    and touch.
    colors are blue and green variants.

  https://www.youtube.com/watch?v=yT7aOK0q17Q

*/


void Menu::draw( Curses & curses )
{

  clear( curses ); // eg. remove text. from last draw

  font(curses, &arial_span_18 ); // font
  color_pair_idx(curses, 0); // blue/white


  Item *item = list_controller.item;
  assert(item );


  // dropdown name
  to(curses, 0, 5);

  if( page_controller.focus)
    effect(curses, 0x01);        // invert
  else
    effect(curses, 0x00);        // normal.   TODO - use enums. CUR_NORMAL | CUR_INVERT

  text(curses, item->name );




  /// draw keys.
  for(unsigned i = 0; i < item->n; ++i)
  {
    to(curses, 0, 6 + i);

    if( i == list_controller.idx && !page_controller.focus ) {
      // printf("setting effect \n");
      // effect(curses, 0x11);        // invert
      effect(curses, 0x01);        // invert
    } else {
      effect(curses, 0x00);        // normal
    }

    text(curses, item->keys[ i  ] );
  }



  /// draw values.
  for(unsigned i = 0; i < item->n; ++i)
  {

    // char buf2[100];
    char buf[100];

    Value *value = & item->values[ i];

    // item->values[ i]. format( item->values[i].value, buf, 100 );
    value->format( value->value, buf, 100 );

    // value_float_format( & item->values[ i ] , buf, 100 );

    // snprintf(buf, 100,  "%smV" , format_float(buf2, 100, 6, item->values[ i ] ));

    to(curses, 10, 6 + i);

    // we have the active item. eg. selected.
    // and also wehter this controller has focus.

    // OK. issue is that we are not clearing the screen I think.

    if( i == list_controller.idx ) {


     /*
      if(page_controller.focus) {

        effect(curses, 0x00);        // normal
        text(curses,  buf);
      }
    */

      if(list_controller.focus) {
        effect(curses, 0x01);        // invert
        text(curses,  buf);
      } else  {

        // draw the text as normal
        effect(curses, 0x00);        // normal
        text(curses,  buf);


        /*
            OK. key here is that we are drawing releative to the dot position.
            idx is relative to dot position. relative to 0 lhs origin.

            if we drew everything from rhs. it should work the same.

        */
        size_t dot_x = dot_position( buf );

        int x =  10  + dot_x - digit_controller.idx;  // digit_controller.idx == element_controller.idx
        // eg. we can fall off the lhs of the screen.
        // we really want to constrain to on screen digits.
        // likewise rhs.
        x = MAX(x, 0);
        assert(x >= 0);

        to(curses, x, 6 + i );

        // issue is that the

        if( element_controller.focus)
          effect( curses, 0x11 );   // invert and blink
        else if (digit_controller.focus )
          effect( curses, 0x01 );   // invert
        // else ; // page_controller // assert( 0 );

        // apply effect at current position
        ch_effect( curses);

      }  {



      }

    } else {
      effect(curses, 0x00);        // normal
      text(curses,  buf);
    }


  }

}




#if 0




void DigitController::draw(Curses &curses)
{
  // don't use this. should be drawn external to the digit controller

  // should call the digit controller. which can draw the value
  char buf[100];

  // grid spacing for text is quite deltaerent than for keypad button spacing.
  color_pair_idx(curses, 0); // blue/white
  //effect(a, 0x01);        // invert
  effect(curses, 0x00);        // normal
  font(curses, &arial_span_18 ); // large font
  to(curses, 0, 4);
  // snprintf(buf, 100, "%f ", this->value);  // our edited value... NOTE. needs extra of active digit etc..


  assert( this->value  );

  format_float(buf, 100, 6, * this->value);
  text(curses,  buf);

  // now we want to place an effect on the active digit.
  // OK. position has to be where the decimal dot is.  and then negative
  // want a function to give us the offset of the '0' character.
  // and the idx bounds also.    perhaps just print into a buffer? and return all of this.
  // no. better to specify prefix/postfix digit count.
  // but also have issue of prefix sign.


  size_t dot_x = dot_position( buf );

  to(curses, 0 + dot_x - this->idx, 4);
  // effect( curses, 0x01 ); // invert.... this just sets the mode.
  effect( curses, 0x11 ); // blink. doesn't seem to work...
  // set the effect at current position
  ch_effect( curses);
  // text(curses,  "x", 1);

}






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
