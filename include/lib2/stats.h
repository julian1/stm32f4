
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
*/

double sum(const float *p, size_t n);
double mean(const float *p, size_t n);


double variance(const float *p, size_t n);

// Need to distinguish... student and global...
// or just use (n-k) as arg
double stddev(const float *p, size_t n);
double stddev2(const float *p, size_t n);

double rms(const float *p, size_t n);


void minmax(const float *p, size_t n, float *min, float *max);



#ifdef __cplusplus
}
#endif


