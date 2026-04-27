
#pragma once


// example only, of possible refactor
error

/*
  using anonymous compositon works well in c99.
  but c++ needs explicit upcasting at the call site
  which is a bit messy.

  struct fsmc_t;      // c99 handles anon composition really nicelyl.
                      // but c++ doesn't....
                      // so pixfmt writer needs explicit  upcas


  get_tear() and even reset() are quite peripheral specific.
  so is having the cd bit.
  so not sure this is a useful abstraction

  also the specific tft_write_data() and vfd_write_data()
  are potentiall specific - not just 8 versus 16 bit. read/write variations

  --------

  so better not to try to add a common base of fsmc_t device functionality
*/

#ifdef __cplusplus
extern "C" {
#endif



typedef struct fsmc_t fsmc_t;

struct fsmc_t
{
  uint32_t  magic;

  uint32_t  addr;     // FMC_MY_BASE |  FMC_A19
  uint32_t  cd;       // command/data bit. FMC_A16.


  void (*port_configure)( fsmc_t *);       //eg .  for gpio reset. and/or tear lines, backlight
  bool (*get_tear)( /* const */ fsmc_t *);
  void (*reset)( fsmc_t *, bool val );

};



static inline void fsmc_port_configure( fsmc_t *fsmc)
{
  // assert(fsmc);
  fsmc->port_configure( fsmc);
}



static inline bool fsmc_get_tear( /* const */ fsmc_t *fsmc)
{
  // assert(fsmc);
  return fsmc->get_tear( fsmc);
}


static inline void fsmc_reset( fsmc_t *fsmc, bool val)
{
  // assert(fsmc);
  fsmc->reset( fsmc, val);
}





#ifdef __cplusplus
}
#endif


