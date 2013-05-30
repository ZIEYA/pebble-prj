#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0xDB, 0xF2, 0x4E, 0x9C, 0x2D, 0x17, 0x42, 0x67, 0xB0, 0xDF, 0x0F, 0x4F, 0xAE, 0x46, 0x62, 0x56 }
PBL_APP_INFO(MY_UUID,
             "zieBeats", "zieya-labo", 1, 0, INVALID_RESOURCE, APP_INFO_WATCH_FACE);
 
Window window;
TextLayer s_text;


#define TIME_STR_BUFFER_BYTES 32
char s_time_str_buffer[TIME_STR_BUFFER_BYTES];

void handle_tick(AppContextRef ctx, PebbleTickEvent *event) {
  string_format_time(s_time_str_buffer, TIME_STR_BUFFER_BYTES, "%I:%M:%S %p", event->tick_time);
  text_layer_set_text(&s_text, s_time_str_buffer);
}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&s_window, "Digital Watch");
  window_stack_push(&s_window, true /* Animated */);

  text_layer_init(&s_text, s_window.layer.frame);
  strcpy(s_time_str_buffer, "");
  text_layer_set_text(&s_text, s_time_str_buffer);
  layer_add_child(&s_window.layer, &s_text.layer);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = SECOND_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
