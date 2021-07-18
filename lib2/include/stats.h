
/*
  we can optimize to compute all of these in one go.
  to avoid recalculation.
*/

double sum(const float *p, size_t n);
double mean(const float *p, size_t n);


double variance(const float *p, size_t n);

// Need to distinguish... student and global...
// or just use (n-1) as arg
double stddev(const float *p, size_t n);
double stddev2(const float *p, size_t n);

double rms(const float *p, size_t n);


void minmax(const float *p, size_t n, float *min, float *max);
