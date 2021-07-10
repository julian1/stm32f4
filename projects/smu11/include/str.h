
// string support



extern char * indent_left(char *s, size_t sz, int indent, const char *string);
extern char * indent_right(char *s, size_t sz, int indent, const char *string);
extern char * snprintf2(char *s, size_t sz, const char *format, ...);
extern char * format_float(char *s, size_t sz, double value, int digits);

extern char * format_bits(char *buf, size_t width, uint32_t value);


