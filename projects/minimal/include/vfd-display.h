
#pragma once




typedef struct data_t data_t;
typedef struct buffer_t buffer_t;
typedef struct vfd_t vfd_t;


#define DISPLAY_VFD_MAGIC   8224371

// think we want to rename this...
// not to conflict with underlying device.

// change name vfd_display. or vfd_display
// not to conflict.

typedef struct vfd_display_t
{
  uint32_t magic;

  buffer_t *buffer;

  vfd_t    *vfd;    // device


} vfd_display_t;




void vfd_display_init( vfd_display_t *vfd, vfd_t *, buffer_t *buffer);


// rename _update_data()
// and make virtual for tests
void vfd_display_update( vfd_display_t *vfd, data_t *data);


