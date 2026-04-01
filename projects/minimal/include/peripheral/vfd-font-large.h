
#pragma once




typedef struct vfd_t vfd_t;



void vfd_font_large_write_proportional( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar);
void vfd_font_large_write_special( vfd_t *vfd, const char *s, uint8_t xpix, uint8_t ychar);


// these funcs go not really belong here

void vfd_clear( vfd_t *vfd);

void vfd_test( vfd_t *vfd);

