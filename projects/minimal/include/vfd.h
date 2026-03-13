
#pragma once


#define VFD_MAGIC   8224371


typedef struct data_t data_t;


typedef struct vfd_t
{
  uint32_t magic;




} vfd_t;




void vfd_init( vfd_t *vfd);

void vfd_upate( vfd_t *vfd, data_t *data);


