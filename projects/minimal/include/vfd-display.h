
#pragma once




typedef struct data_t data_t;
typedef struct buffer_t buffer_t;
typedef struct vfd_t vfd_t;



// fixme, rename VFD_DISPLAY
#define VFD_DISPLAY_MAGIC   8224371




typedef struct vfd_display_t
{
  uint32_t magic;

  buffer_t *buffer;

  vfd_t    *vfd;



/*
  can make polymorphic for strategy-like pattern. and to test separate functions
  as needed

  void (*update)( tft_display_t *);
  void (*update_data)( tft_display_t *, data_t *data);
  void (*update_500ms)( tft_display_t *);
*/

} vfd_display_t;




void vfd_display_init( vfd_display_t *vfd, vfd_t *, buffer_t *buffer);


void vfd_display_update( vfd_display_t *vfd_display) ;
void vfd_display_update_data( vfd_display_t *vfd, data_t *data);
void vfd_display_update_500ms( vfd_display_t *vfd);


