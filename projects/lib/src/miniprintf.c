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
#include <miniprintf.h>

/*********************************************************************
 * Internal structure for I/O
 *********************************************************************/

struct s_mini_args {
	void	(*putc)(char,void *);	// The putc() function to invoke
	void 	*argp;			// Associated data struct
};

typedef struct s_mini_args miniarg_t;	// Abbreviated ref to s_mini_args

/*********************************************************************
 * Internal: Write string msg until null byte, to the I/O
 *           routine described by s_mini_args.
 *********************************************************************/

static void
mini_write(miniarg_t *mini,const char *msg) {
	char ch;

	while ( (ch = *msg++) != 0 )
		mini->putc(ch,mini->argp);
}

/*********************************************************************
 * Internal: Pad % field to width, give text buffer.
 *********************************************************************/

static void
mini_pad(miniarg_t *mini,char pad,int width,const char *text) {
	int slen;

	if ( width > 0 ) {
		slen = strlen(text);

		for ( width -= slen; width > 0; --width )
			mini->putc(pad,mini->argp);
	}
}

/*********************************************************************
 * Internal: mini_printf() engine.
 *********************************************************************/


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



static void
internal_vprintf(miniarg_t *mini,const char *format,va_list arg)
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
			mini->putc(ch,mini->argp);
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
			mini->putc((char)vint,mini->argp);
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
			mini_pad(mini,pad,width,bptr);
			mini_write(mini,bptr);
			break;




		case 'd':		/* Decimal format */
			if ( !longf ) {
				vint = va_arg(arg,int);
				if ( vint < 0 ) {
					mini->putc('-',mini->argp);
					vint = -vint;
				} else if ( sgn == '+' )
					mini->putc(sgn,mini->argp);
				bptr = buf + sizeof buf;
				*--bptr = 0;
				do	{
					*--bptr = vint % 10 + '0';
					vint /= 10;
				} while ( vint != 0 );
			} else	{
				vlong = va_arg(arg,long);
				if ( vlong < 0 ) {
					mini->putc('-',mini->argp);
					vlong = -vlong;
				} else if ( sgn == '+' )
					mini->putc(sgn,mini->argp);
				bptr = buf + sizeof buf;
				*--bptr = 0;
				do	{
					*--bptr = vlong % 10 + '0';
					vlong /= 10;
				} while ( vlong != 0 );
			}
			mini_pad(mini,pad,width,bptr);
			mini_write(mini,bptr);
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
          mini->putc( uint & (1 << i) ? '1' : '0', mini->argp);
      }
      break;

    // JA float/double
		case 'f':
      {
        // very basic float
        // calling va_arg really doesn't seem to be working, unless use double on both sides.
				// float x = va_arg(arg,double);    // fails.
				double x = va_arg(arg,double);

        // use width for trailing digits prec.
        unsigned prec = width != 0 ? width : 7;

        int intpart = (int)x;
        int fracpart = (int)((x- intpart) * int_pow(10, prec));   // number of digits of precision

        // handle negative
        if(intpart<0) {
          intpart *= -1;
          fracpart *= -1;
          mini->putc('-',mini->argp);
        }

        ////////////////////
        // do int part
				bptr = buf + sizeof buf;
				*--bptr = 0;
				do	{
					*--bptr = intpart % 10u + '0';
					intpart /= 10u;
				} while (intpart != 0 );
        mini_write(mini,bptr);

        // dot
        mini->putc('.',mini->argp);

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
        mini_write(mini,bptr);

        break;
      }



		case 'p':		/* Pointer: assumes pointer is sizeof(unsigned) */
			mini_write(mini,"0x");
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
			mini_pad(mini,pad,width,bptr);
			mini_write(mini,bptr);
			break;

		case 's':		/* String format */
			sptr = va_arg(arg,const char *);
			if ( sgn != '-' )
				mini_pad(mini,pad,width,sptr);
			mini_write(mini,sptr);
			if ( sgn == '-' )
				mini_pad(mini,pad,width,sptr);
			break;

		case '%':		/* "%%" outputs as "%" */
			mini->putc(ch,mini->argp);
			break;

		default:		/* Unsupported stuff here */
			mini->putc('%',mini->argp);
			mini->putc('?',mini->argp);
			mini->putc(ch,mini->argp);
		}
	}
}

/*********************************************************************
 * s_internal trackes the count of bytes output:
 *********************************************************************/

struct s_internal {
	void (*putc)(char);	/* User's putc() routine to be used */
	unsigned count;		/* Bytes output */
	unsigned cooked : 1;	/* When true, '\n' also emits '\r' */
};

static void
mini_putc(char ch,void *argp) {
	struct s_internal *internp = (struct s_internal *)argp;

	internp->putc(ch);	/* Perform I/O */
	++internp->count;	/* Count bytes out */

	if ( ch == '\n' && internp->cooked != 0 ) {
		/* In cooked mode, issue CR after LF */
		internp->putc('\r');
		++internp->count;	/* Count CR */
	}
}

/*********************************************************************
 * Internal: Perform cooked/uncooked printf()
 *********************************************************************/

static int
mini_vprintf0(void (*putc)(char),int cooked,const char *format,va_list args) {
	miniarg_t mini;
	struct s_internal intern;

	intern.putc = putc;		/* User's putc() routine to be used */
	intern.count = 0u;		/* Byte counter */
	intern.cooked = !!cooked; 	/* True if LF to add CR */

	mini.putc = mini_putc;		/* Internal interlude routine */
	mini.argp = (void *)&intern; 	/* Ptr to internal struct */

	internal_vprintf(&mini,format,args);
	return intern.count;		/* Return byte count */
}

/*********************************************************************
 * External: Perform cooked mode printf()
 *********************************************************************/

int
mini_vprintf_cooked(void (*putc)(char),const char *format,va_list args) {
	return mini_vprintf0(putc,1,format,args);
}

/*********************************************************************
 * External: Perform uncooked (as is) printf()
 *********************************************************************/

int
mini_vprintf_uncooked(void (*putc)(char),const char *format,va_list args) {
	return mini_vprintf0(putc,0,format,args);
}

/*********************************************************************
 * Sprintf
 *********************************************************************/

struct s_mini_sprintf {
	char	*buf;			/* Ptr to output buffer */
	unsigned maxbuf;		/* Max bytes for buffer */
	char	*ptr;			/* Ptr to next byte */
};

static void
mini_sputc(char ch,void *argp) {
	struct s_mini_sprintf *ctl = (struct s_mini_sprintf *)argp;

	if ( (unsigned)(ctl->ptr - ctl->buf) >= ctl->maxbuf )
		return;
	*ctl->ptr++ = ch;
}

/*********************************************************************
 * External: sprintf() to buffer (not cooked)
 *********************************************************************/

int
mini_snprintf(char *buf,unsigned maxbuf,const char *format,...) {
	miniarg_t mini;			/* printf struct */
	struct s_mini_sprintf ctl;	/* sprintf control */
	va_list args;			/* format arguments */
	unsigned count;			/* Return count */

	mini.putc = mini_sputc;		/* Internal routine */
	mini.argp = (void *)&ctl;	/* Using ctl to guide it */

	ctl.ptr = ctl.buf = buf;	/* Destination for data */
	ctl.maxbuf = maxbuf;		/* Max size in bytes */

	va_start(args,format);
	internal_vprintf(&mini,format,args);
	va_end(args);

	count = (unsigned)(ctl.ptr - ctl.buf); /* Calculate count */
	mini_sputc(0,&ctl);		/* Null terminate output if possible */
	return count;			/* Return formatted count */
}

/* End miniprintf.c */
