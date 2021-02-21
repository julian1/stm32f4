

#include <stdio.h>   // printf
#include <stddef.h>   // size_t
// #include <strings.h>   // bzero deprecated
#include <string.h>   // memset

// does one even need an array of past values...
// use an array

// it should be easy enough to verify against an implementation  - that calculates everything explicitly ...
// eg. using a vector<float>




// dt = 1 at all times.


typedef struct PID {
  int   j;      // what about i overflowing - will need to handle -
  int   len;
  float *buf;

  // error terms
  float p; // should be pe, ie, di
  float i;
  float d;

  // coeff - could put in separate struct
  float kp;
  float ki;
  float kd;

} PID;


void pid_init( PID *pid, float *buf, size_t len)
{
  memset( pid, 0, sizeof(PID));
  pid->buf = buf;
  pid->len = len;

  // 0 as initial starting term is reasonable...
  memset( pid->buf, 0, pid->len * sizeof(float));
}

void pid_set( PID *pid, float kp, float ki, float kd)
{
  pid->kp = kp;
  pid->ki = ki;
  pid->kd = kd;
}

/*
  to make this fast, 
  probabl easier to just keep a temp variable around - for previous value - rather than indexing?
  and clamp integral.
*/

void pid_update(PID *pid, float set_point, float process_var)
{
  float *buf = pid->buf;
  int   len = pid->len;

  /////////////

  float e = set_point - process_var;
  ++pid->j;   // calc new index position to store error val

  // handle loop around
  if(pid->j == len)
    pid->j = 0;

  int j = pid->j;
  int prev = j == 0 ? len - 1 : j - 1;

  printf( "j is %d, prev is %d\n", j, prev );

  // proportional eor
  pid->p = e;

  // integral = current - outgoing error term + new error term
  printf( "outgoing %f\n", (buf[ j ] / len));
  printf( "new      %f\n", (e / len) );

  // could remove a div - here by only calclating / len when getting the control var.
  pid->i = pid->i - (buf[ j ] / len) + (e / len);

  // derivative
  pid->d = e - buf[ prev ]; // prev, not oldest.

  // update the e
  buf[ j ] = e;
}


float pid_control_var(PID *pid)
{

  return (pid->kp * pid->p) + (pid->ki * pid->i) + (pid->kd * pid->d);
}

/*
  alternative - does not use a window - but would need clamp values on error term...
  https://www.eevblog.com/forum/microcontrollers/c-code-implementation-of-pid-in-microcontroller/

  another without window,
  http://robotsforroboticists.com/pid-control/

  frequency can be changed - by including dt term. instead of changing the array size.

  control rate - doesn't have to be that fast. eg. duty oscillator can be 100kHz. but feedback could be 1000kHz
  or even slower - depending on power supply surge requirements - and caps

  clamps not needed.
    integral is limited by window.
    others are limited by terms

  pid - kind of - present, past (centering), future (dampening).

  very good explanation/intuition for affect of terms
  https://www.youtube.com/watch?v=0vqWyramGy8

  real time means. no queue inputs. 

*/


int main()
{
  int   i;
  float buf[10];
  PID   pid;

  pid_init(&pid, buf, 10 );
  pid_set(&pid, 0, 1, 0);

  for(i = 0; i < 500; ++i) {

    pid_update(&pid, 123, 124 );
    printf( "%d %f\n", i, pid_control_var(&pid));
    printf("\n");
  }

  return 0;
}

