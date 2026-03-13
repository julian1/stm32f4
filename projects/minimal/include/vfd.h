
#pragma once




typedef struct data_t data_t;
typedef struct buffer_t buffer_t;


#define VFD_MAGIC   8224371

typedef struct vfd_t
{
  uint32_t magic;

  buffer_t *buffer;



} vfd_t;




void vfd_init( vfd_t *vfd, buffer_t *buffer);

void vfd_upate( vfd_t *vfd, data_t *data);


