


#pragma once

#define ENVIRONMENT_MAGIC 87112371

typedef struct environment_t
{

  // it is ok. to have only a single var here.
  // much easier to manage this struct pointer than a pointer to the int/double


  uint32_t      magic;

  uint32_t      line_freq;

} environment_t;





