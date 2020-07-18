

#include <stdio.h>   // printf
#include <stddef.h>   // size_t
// #include <strings.h>   // bzero deprecated
#include <string.h>   // memset

// does one even need an array of past values...
// use an array

// it should be easy enough to verify against an implementation  - that calculates everything explicitly ...
// eg. using a vector<float>


// sensible values - are always going to be around 0 and 1. representing full range correction.
// clamp can also be around 1. or maybe lower 0.5

// we don't actually even need events - for reading rotary input - just have a general loop - that
// reads the rotary position, and adjusts the position.
// we might if want to change lcd.

// dt = 1 at all times.


typedef struct PID {

  float prev;
  float clamp;
  float i;

  float kp;
  float ki;
  float kd;

} PID;


void pid_init( PID *pid)
{
  memset( pid, 0, sizeof(PID));
}

void pid_set( PID *pid, float kp, float ki, float kd, float clamp)
{
  pid->kp = kp;
  pid->ki = ki;
  pid->kd = kd;
  pid->clamp = clamp;
}

// update - should yield the damn value - not store it...

float pid_update(PID *pid, float set_point, float process_var)
{
  float e = set_point - process_var;

  // proportional
  float p = e;

  // integral
  pid->i = pid->i + e;

  // clamp
  if(pid->i > pid->clamp)
    pid->i = pid->clamp;

  else if(pid->i < pid->clamp)
    pid->i = - pid->clamp;

  // derivative - inverse to time
  float d = e - pid->prev;

  // update the prev
  pid->prev = e;

  float var = (pid->kp * p) + (pid->ki * pid->i) + (pid->kd * d);
  return var;

}


/*
  very good - discussion 3 pages,
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

#if 0
int main()
{
  int   i;
  float buf[10];
  PID   pid;

  pid_init(&pid);
  pid_set(&pid, 0, 1, 0);

  for(i = 0; i < 500; ++i) {

    pid_update(&pid, 123, 124 );
    printf( "%d %f\n", i, pid_control_var(&pid));
    printf("\n");
  }

  return 0;
}
#endif

