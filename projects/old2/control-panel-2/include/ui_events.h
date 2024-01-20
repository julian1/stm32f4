


#ifndef UI_EVENTS_H
#define UI_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif


// struct CBuf;
typedef struct CBuf CBuf;

void ui_events_init(  CBuf *ui_events_in_ );


enum ui_events_t
{
  ui_events_rotary_change = 23,
  ui_events_button_rotary = 24,
  ui_events_button_left = 25,
  ui_events_button_right = 26

};


#endif // UI_EVENTS_H

#ifdef __cplusplus
}
#endif



