
/*
  The STM32F4 only supports 32 bit doubleing point in hardware, so these functions
  must be done in software and will take many cycles to execute. To ensure that
  all calculations are done in 32 bit you should define all your doubleing point
  numbers (including constants) as type double.

  https://electronics.stackexchange.com/questions/231705/stm32f4-doubleing-point-instructions-too-slow

  ------------

  https://www.allaboutcircuits.com/technical-articles/how-standard-deviations-relates-rms-values/

  With RMS, we square the data points; with standard deviation, we square the
  difference between each data point and the mean.

  First, standard deviation gives us the “AC coupled” RMS amplitude of a
  waveform: we can calculate standard deviation when the DC offset of a signal is
  irrelevant, and this gives us the RMS amplitude of only the AC portion.

  -------

  Use double everywhere.
  A 4-byte float (IEEE 754 single-precision) is accurate to approximately 7 decimal digits.


*/

/*
  code requires double prec accuracy.
  through some of these calculations to carry meaninful precision.
  not quite sure where though

  the square() must be double...

*/


#include <math.h>   // sqrt,sqrtf
#include <assert.h>
// #include <double.h>   // FLT_MAX, FLT_MIN


#include "stats.h"


static inline double square(double x)
{
  // force double prec. must be double prec to carry enough prec.
  // return pow(x, 2);
  return x * x;
}


double sum(const double *p, size_t n)
{
  /*
    for initializer is valid C99
    for( const double *end = p + n; p < end; ++p)
  */

  double sum = 0;
  const double *end = p + n;
  while(p < end)
    sum += *p++;

  return sum;
}


static double sumX2(const double *p, size_t n)
{
  double sum = 0;
  const double *end = p + n;
  while(p < end)
    sum += square(*p++);

  return sum;
}

// mean, stddeve, rms are all the same values?????
// so if they're the same then we return 0...

double mean(const double *p, size_t n)
{
  return sum(p, n) / n;
}


double variance(const double *p, size_t n)
{
  double m = mean(p, n);
  double sum = 0;

  const double *end = p + n;
  while(p < end)
    sum += square(*p++ - m);

  return sum / n;
}

// population. should probably be sample.

double stddev(const double *p, size_t n)
{
  return sqrt(variance(p, n));
}


double stddev2(const double *p, size_t n)
{
  // alternate calc approach. also works.
  double j = (sumX2(p, n) / n) - square(sum(p, n) / n);
  return sqrt( j );
}



double rms(const double *p, size_t n)
{
  // seems like a problematic calculation if DC not blocked.
  return sqrt(sumX2(p, n) / n);
}



/*
#define min(a, b) ((a) < (b) ? (a) : (b))
*/

void minmax(const double *p, size_t n, double *min, double *max)
{
  assert(n > 0);
  *min = *p;
  *max = *p;

  for(const double *end = p + n; p < end; ++p)
  {
    if(*p > *max)
      *max = *p;

    if(*p < *min)
      *min = *p;
  }
}








#if 0
void push(double *p, size_t n, size_t *idx, double val)
{
  // don't advance if idx == n, to support overflow detect
  // assert(*idx < n);
  if(*idx < n) {
    p[*idx] = val;
    ++(*idx);
  }
}

#endif


#if 0
// we need to set up an interrupt for a timer. eg. 1 sec.
// so the interrupt for adc just pushes values.
// then interupt for 1sec. calculates noise etc.

void stat_test()
{
  double buf [ 10];
  size_t n = 0;

  // eg. add values 3, 4, 5
  push(buf, 10, &n, 3);
  push(buf, 10, &n, 4);
  push(buf, 10, &n, 5);
  // push(buf, &n, 5);
  // push(buf, &n, 5);

  printf("mean   %f\n",  mean(buf, n));    // 4.000000
  printf("stddev %f\n",  stddev(buf, n));  // 0.816496
  printf("rms    %f\n",  rms(buf, n));     // 4.082
  printf("vmax   %f\n",  rms(buf, n) / 0.7071 );     // 4.082... 0.7071 == sqrt(0.5)
}


// so if these were measured over a time period...
// then we just need to do the



int main()
{
  stat_test();

  return 0;
}

#endif

