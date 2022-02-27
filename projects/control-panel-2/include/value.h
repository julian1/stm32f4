
#pragma once

typedef void (*edit_t)( void *, unsigned idx, int delta );
typedef void (*copy_t)( const void *, void *dst, size_t sz );
typedef void (*format_t)( const void *, char *buf, size_t sz);
typedef  bool (*validate_t)( void *) ;

struct Value  // should have a different name
{
  void *val;    // rename to val. because using value everywhere.
                // or name to var. indicating variant.

  // actually separating the controller functions from the values - means less repetition. and filling things in.
  edit_t    edit;
  copy_t    copy;
  format_t  format ;
  validate_t validate ;
  // name_t name;

  // bool no_element_controller. // No. should advertise edit capabilities via a function. eg. return value with bitvalues set.

  Value(
    void *val_,

    edit_t edit_,
    copy_t copy_,
    format_t format_,
    validate_t validate_
  ) :
    val(val_),
    edit(edit_),
    copy(copy_),
    format(format_),
    validate(validate_)
  { }

};

// this will have to take the argument
// and modify it in place
void value_float_edit(double *x, int idx, int amount);
void value_float_copy( const double *x, void *dst, size_t sz );
void value_float_format( const double *x, char *buf, size_t sz);
void value_float_format2( const double *x, char *buf, size_t sz);


void value_bool_edit(bool *x, int idx, int amount);
void value_bool_copy( const bool *x, void *dst, size_t sz );
void value_bool_format( const bool *x, char *buf, size_t sz);


