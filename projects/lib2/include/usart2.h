

typedef struct A
{
  char *p;
  size_t sz;
  size_t wi;
  size_t ri;
} A;

void init(A *a, char *p, size_t sz);
void write(A *a, char val);
int32_t read(A *a);


void usart_setup(A *output, A *input);


void usart_output_update(void);
void usart_input_update(void);

void usart_output_flush(void);


