/* Minimal printf() facility for MCUs
 * Warren W. Gay VE3WWG,  Sun Feb 12 2017
 *
 * This work is placed in the public domain. No warranty, or guarantee
 * is expressed or implied. When uou use this source code, you do so
 * with full responsibility and at your own risk.
 */
#include <string.h>
#include <stdbool.h>
// #include <stdint.h> // JA size_t
// #include <math.h> // JA pow10l
#include <miniprintf2.h>



static void
mini_write(
	void	(*putc)(void *, char),	// The putc() function to invoke
	void 	*ctx,			          // Associated data struct
  const char *msg)
{
	char ch;

	while ( (ch = *msg++) != 0 ) {
    // JA
		// mini->putc(ch,mini->ctx);
    putc(ctx, ch);
  }
}

static void
mini_pad(
	void	(*putc)(void *, char),	// The putc() function to invoke
	void 	*ctx,			          // Associated data struct
  char pad,int width,const char *text)
{
	int slen;

	if ( width > 0 ) {
		slen = strlen(text);

		for ( width -= slen; width > 0; --width ) {
      // JA
			// mini->putc(pad,mini->ctx);
      putc(ctx, pad);
    }
	}
}



// https://stackoverflow.com/questions/29787310/does-pow-work-for-int-data-type-in-c

static int int_pow(int base, int exp)
{
  int result = 1;
  while (exp)
  {
      if (exp % 2)
         result *= base;
      exp /= 2;
      base *= base;
  }
  return result;
}



void internal_vprintf(
	void	(*putc)(void *, char),	// the putc() function to invoke
	void 	*ctx,			              // associated data struct
  const char *format,
  va_list arg)
{
	char ch, pad, sgn;	/* Current char, pad char and sign char */
	int vint, width;	/* Integer value to print and field width */
	long vlong;
	unsigned uint;		/* Unsigned value to print */
	unsigned long ulong;	/* Unsigned long value to print */
	const char *sptr;	/* String to print */
	char buf[40], *bptr;	/* Formatting buffer for int/uint */
	char ccase = 0;
	bool longf;		/* True when %ld */

	while ( (ch = *format++) != 0 ) {
		if ( ch != '%' ) {
			/* Non formatting field: copy as is */
      // JA
			// mini->putc(ch,mini->ctx);

#if 1
      if(ch == '\n')
        putc(ctx, '\r' );
#endif

			putc(ctx, ch);
			continue;
		}

		/*
		 * Process a format item:
		 */
		pad = ' ';	/* Default pad char is space */
		sgn = 0;	/* Assume no format sign char */
		ch = *format++;	/* Grab next format char */

		if ( ch == '+' || ch == '-' ) {
			sgn = ch;	/* Make note of format sign */
			ch = *format++;	/* Next format char */
		}

		if ( ch == '0' ) {
			pad = ch;	/* Pad with zeros */
			ch = *format++;
		}

		/*
		 * Extract width when present:
		 */
		for ( width = 0; ch && ch >= '0' && ch <= '9'; ch = *format++ )
			width = width * 10 + (ch & 0x0F);

		if ( !ch )
			break;		/* Exit loop if we hit end of format string (in error) */

		if ( ch == 'l' ) {
			longf = true;
			ch = *format++;
		}

		/*
		 * Format according to type: d, x, or s
		 */
		switch ( ch ) {
		case 'c':
			if ( !longf )
				vint = va_arg(arg,int);
			else	vint = va_arg(arg,long);
      // JA
			// mini->putc((char)vint,mini->ctx);
			putc(ctx, (char)vint);
			break;

		case 'u':		/* Unsigned decimal */
			if ( !longf ) {
				uint = va_arg(arg,unsigned);
				bptr = buf + sizeof buf;
				*--bptr = 0;
				do	{
					*--bptr = uint % 10u + '0';
					uint /= 10u;
				} while ( uint != 0 );
			} else	{
				ulong = va_arg(arg,unsigned long);
				bptr = buf + sizeof buf;
				*--bptr = 0;
				do	{
					*--bptr = ulong % 10u + '0';
					ulong /= 10u;
				} while ( ulong != 0 );
			}

      // JA
			// mini_pad(mini,pad,width,bptr);
			// mini_write(mini,bptr);
			mini_pad(putc, ctx, pad,width,bptr);
			mini_write(putc, ctx, bptr);
			break;




		case 'd':		/* Decimal format */
			if ( !longf ) {
				vint = va_arg(arg,int);
				if ( vint < 0 ) {
					// mini->putc('-',mini->ctx); JA
          putc(ctx, '-');
					vint = -vint;
				} else if ( sgn == '+' )
					// mini->putc(sgn,mini->ctx); JA
          putc(ctx, '+');
				bptr = buf + sizeof buf;
				*--bptr = 0;
				do	{
					*--bptr = vint % 10 + '0';
					vint /= 10;
				} while ( vint != 0 );
			} else	{
				vlong = va_arg(arg,long);
				if ( vlong < 0 ) {
					// mini->putc('-',mini->ctx);
          putc(ctx, '-');
					vlong = -vlong;
				} else if ( sgn == '+' )
					// mini->putc(sgn,mini->ctx);
          putc(ctx, '+');
				bptr = buf + sizeof buf;
				*--bptr = 0;
				do	{
					*--bptr = vlong % 10 + '0';
					vlong /= 10;
				} while ( vlong != 0 );
			}
			// mini_pad(mini,pad,width,bptr); // JA
			// mini_write(mini,bptr);
			mini_pad(putc, ctx, pad,width,bptr);
			mini_write(putc, ctx, bptr);
			break;

    // JA octal would be nice.

    // JA binary
    /* eg.
        usart_printf("test %b\n", 0x01 );
        usart_printf("test %16b\n", 0xffff );
    */
    case 'b':
      uint = va_arg(arg,unsigned);
      if( width == 0 )
        width = 8;

      for(int i = width - 1; i >= 0; --i ) {
          // mini->putc( uint & (1 << i) ? '1' : '0', mini->ctx); JA
          putc(ctx, uint & (1 << i) ? '1' : '0');
      }
      break;

    // JA float/double
		case 'f':
      {
        // very basic float
        // calling va_arg really doesn't seem to be working, unless use double on both sides.
				// float x = va_arg(arg,double);    // fails.
				double x = va_arg(arg,double);

        // TODO change to... to ensure we do everything with 32bit. for hardware support.
        // float x = (double) va_arg(arg,double);

        // use width to determine trailing digits prec.
        unsigned prec = width != 0 ? width : 6;

        int intpart = (int)x;
        int fracpart = (int)((x- intpart) * int_pow(10, prec));   // number of digits of precision

        // handle negative
        if(x < 0.0) {
          intpart *= -1;
          fracpart *= -1;
          // mini->putc('-',mini->ctx); JA
          putc(ctx, '-');
        }

        ////////////////////
        // do int part
				bptr = buf + sizeof buf;
				*--bptr = 0;
				do	{
					*--bptr = intpart % 10u + '0';
					intpart /= 10u;
				} while (intpart != 0 );
        // mini_write(mini,bptr); JA
			  mini_write(putc, ctx, bptr);

        // dot
        // mini->putc('.',mini->ctx); JA
        putc(ctx, '.');

        ////////////////////
        // do frac part

				bptr = buf + sizeof buf;
				*--bptr = 0;

        bool gotnonzero = 0;

        // must be to fixed loop/ not while
        for(unsigned i = 0; i < prec; ++i)
        {
          // take care to truncate trailing zeros
          char ch1 = fracpart % 10u + '0';
          if(ch1 != '0')
            gotnonzero = true;

          if(gotnonzero == true)
					  *--bptr = ch1;

					fracpart /= 10u;
        }
        // mini_write(mini,bptr); JA
			  mini_write(putc, ctx, bptr);
        break;
      }



		case 'p':		/* Pointer: assumes pointer is sizeof(unsigned) */
			// mini_write(mini,"0x"); JA
      mini_write(putc, ctx, "0x");
			/* Fall Thru */
		case 'x':		/* Hexadecimal format */
			ccase = 0x20;	/* Flip case */
			/* Fall Thru */
		case 'X':
			if ( !longf ) {
				uint = va_arg(arg,unsigned);
				bptr = buf + sizeof buf;
				*--bptr = 0;
				do	{
					ch = uint & 0x0F;
					*--bptr = ch + (ch <= 9 ? '0' : ('A'^ccase)-10);
					uint >>= 4;
				} while ( uint != 0 );
			} else	{
				ulong = va_arg(arg,unsigned long);
				bptr = buf + sizeof buf;
				*--bptr = 0;
				do	{
					ch = ulong & 0x0F;
					*--bptr = ch + (ch <= 9 ? '0' : ('A'^ccase)-10);
					ulong >>= 4;
				} while ( ulong != 0 );
			}
			// mini_pad(mini,pad,width,bptr); // JA
			// mini_write(mini,bptr);
			mini_pad(putc, ctx, pad,width,bptr);
			mini_write(putc, ctx, bptr);
			break;

		case 's':		/* String format */
			sptr = va_arg(arg,const char *);
			if ( sgn != '-' )
				// mini_pad(mini,pad,width,sptr); // JA
			  mini_pad(putc, ctx, pad,width,sptr);
			// mini_write(mini,sptr);
			mini_write(putc, ctx, sptr);
			if ( sgn == '-' )
				// mini_pad(mini,pad,width,sptr);
			  mini_pad(putc, ctx, pad,width,sptr);
			break;

		case '%':		/* "%%" outputs as "%" */
			// mini->putc(ch,mini->ctx);
      putc(ctx, ch);
			break;

		default:		/* Unsupported stuff here */
			// mini->putc('%',mini->ctx);
			// mini->putc('?',mini->ctx);
			// mini->putc(ch,mini->ctx);

      putc(ctx, '%');
      putc(ctx, '?');
      putc(ctx, ch);
		}
	}
}

/*

  we can change this...

struct Buffer
{
  putc() should handle translation '\n' to \r\n

	void	(*putc)(void *, char),
	void 	*ctx,

  bool lineBuffer;
  void (*sync)() ;
}


*/

int
mini_printf(
	void	(*putc)(void *, char),
	void 	*ctx,
  const char *format,...)
{
  /*
    - we could configure this to do line buffering
    if we really wanted. just call usart_sync_flush()

    ctx can be a more complicated structure...
    in fact this function should perhaps be elsewhere.
  */

	va_list args;			/* format arguments */

	va_start(args,format);
	internal_vprintf(putc, ctx, format,args);
	va_end(args);

  return 0;
}


