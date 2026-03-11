

#pragma once



#include <stdbool.h>


// from /test
bool app_test01( app_t *app , const char *cmd);
bool app_test02( app_t *app , const char *cmd);


// adc refmux test.
bool app_test08( app_t *app , const char *cmd);

// sa/adc test
bool app_test09( app_t *app , const char *cmd);


// dcvsource
bool app_test10( app_t *app , const char *cmd);
bool app_test11( app_t *app , const char *cmd);

// input mux tests
bool app_test12( app_t *app , const char *cmd);     // formerly test05.
bool app_test14( app_t *app , const char *cmd);
bool app_test15( app_t *app , const char *cmd);


// TODO. remove yield arguments
// point of passing app is to have the context available
// add - app_test_yield, app_test_yield_ctx;
// to be controllable.
bool app_test20( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;

bool app_test40( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;
bool app_test41( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;
bool app_test42( app_t *app, const char *cmd, void (*yield)( void * ), void * yield_ctx) ;



bool app_test50( app_t *app , const char *cmd);
bool app_test51( app_t *app , const char *cmd);
bool app_test52( app_t *app , const char *cmd);





