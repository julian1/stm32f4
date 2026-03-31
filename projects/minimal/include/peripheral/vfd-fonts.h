
#pragma once


/*
  peripheral independent code,
  unrelated to specific vfd instance
  except we do not bother to pass an instance argument at the moment
*/

typedef struct vfd_t vfd_t;







// need a better name to distinguish this
void vfd_write_string2( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar);


////////////


void vfd_write_bitmap_string_proportional( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar);
void vfd_write_bitmap_string_mono( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar);


void vfd_clear( vfd_t *vfd);

// todo move
void vfd_test( vfd_t *vfd);

