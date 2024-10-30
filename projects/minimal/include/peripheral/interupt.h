

/* interupt can be abstract, and un-associated. 

  eg. systick handler.
  or spi .
  or other source.

  ju


  eg. like
  void spi1_port_interupt_handler_set( void (*pfunc_)(void *), void *ctx);

  But should be a structure. and simple to have several sources..



*/
