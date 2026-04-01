
#pragma once


/*
  peripheral independent code,
  unrelated to specific vfd instance
  except we do not bother to pass an instance argument at the moment
*/

typedef struct vfd_t vfd_t;





void vfd_font_small_write( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar);

void vfd_font_small_write_special( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar);


