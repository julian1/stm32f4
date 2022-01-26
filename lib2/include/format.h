

#ifdef __cplusplus
extern "C" {
#endif


// string support

// should rename to format.h.



extern char * indent_left(char *s, size_t sz, int indent, const char *string);
extern char * indent_right(char *s, size_t sz, int indent, const char *string);
extern char * snprintf2(char *s, size_t sz, const char *format, ...);

// change name format_double...
extern char * format_float(char *s, size_t sz, int digits, double value);
extern char * format_float_with_commas(char *s, size_t sz, int digits, double value);

extern char * format_bits(char *buf, size_t width, uint32_t value);


#ifdef __cplusplus
}
#endif


