/*
  string support
  rename to str_format.h.
*/

#pragma once


#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>  // uint32_t etc
#include <stddef.h>  // size_t


char * str_trim_whitespace_inplace( char *cmd );


char * str_indent_left(char *s, size_t sz, int indent, const char *string);
char * str_indent_right(char *s, size_t sz, int indent, const char *string);
char * snprintf2(char *s, size_t sz, const char *format, ...);

// change name format_double...
char * str_format_float(char *s, size_t sz, int digits, double value);
char * str_format_float_with_commas(char *s, size_t sz, int digits, double value);

char * str_format_bits(char *buf, size_t width, uint32_t value);


#ifdef __cplusplus
}
#endif


