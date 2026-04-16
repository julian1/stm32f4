
#pragma once




typedef struct data_t data_t;
typedef struct buffer_t buffer_t;
typedef struct vfd_t vfd_t;



// fixme, rename VFD_DISPLAY
#define VFD_DISPLAY_MAGIC   8224371




typedef struct display_vfd_t
{
  uint32_t magic;

  buffer_t *buffer;

  vfd_t    *vfd;



/*
  can make polymorphic for strategy-like pattern. and to test separate functions
  as needed

  void (*update)( display_tft_t *);
  void (*update_data)( display_tft_t *, data_t *data);
  void (*update_500ms)( display_tft_t *);
*/

} display_vfd_t;




void display_vfd_init( display_vfd_t *vfd, vfd_t *, buffer_t *buffer);


void display_vfd_update( display_vfd_t *display_vfd) ;
void display_vfd_update_data( display_vfd_t *vfd, data_t *data);
void display_vfd_update_500ms( display_vfd_t *vfd);


