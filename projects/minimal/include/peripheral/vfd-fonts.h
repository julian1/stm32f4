
#pragma once


/*
  peripheral independent code,
  unrelated to specific vfd instance
  except we do not bother to pass an instance argument at the moment
*/

typedef struct vfd_t vfd_t;

void vfd_write_bitmap_string2( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar );

void vfd_write_string2( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar );


void vfd_clear( vfd_t *vfd);

// todo move
void vfd_test( vfd_t *vfd);

