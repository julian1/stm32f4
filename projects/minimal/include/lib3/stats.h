
#pragma once

#ifdef __cplusplus
extern "C" {
#endif


/*
  can optimize and compute all these funcs at once (mean, std,dev) etc.
  and avoid recalculation.
  --------------------

  this stuff might be faster using double. to avoid the conversion.
  even if we already do calculations with double

  Use double everywhere.
  A 4-byte float (IEEE 754 single-precision) is accurate to approximately 7 decimal digits.
*/

double sum(const double *p, size_t n);
double mean(const double *p, size_t n);


double variance(const double *p, size_t n);

// Need to distinguish... student and global...
// or just use (n-k) as arg
double stddev(const double *p, size_t n);
double stddev2(const double *p, size_t n);

double rms(const double *p, size_t n);


void minmax(const double *p, size_t n, double *min, double *max);



#ifdef __cplusplus
}
#endif


