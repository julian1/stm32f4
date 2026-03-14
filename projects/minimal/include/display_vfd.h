
#pragma once




typedef struct data_t data_t;
typedef struct buffer_t buffer_t;
typedef struct vfd_t vfd_t;


#define DISPLAY_VFD_MAGIC   8224371

// think we want to rename this...
// not to conflict with underlying device.

// change name vfd_display. or display_vfd
// not to conflict.

typedef struct display_vfd_t
{
  uint32_t magic;

  buffer_t *buffer;

  vfd_t    *vfd;    // device


} display_vfd_t;




void display_vfd_init( display_vfd_t *vfd, vfd_t *,  buffer_t *buffer);

void display_vfd_update( display_vfd_t *vfd, data_t *data);


