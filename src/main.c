#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_connection_layer;
static GFont *s_system_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
//static InverterLayer *s_inverter_layer;

static void handle_battery(BatteryChargeState charge_state) {
  // if (charge_state.is_charging) {
  //   text_layer_set_text(s_battery_layer, "\ue600");
  // } else {
  //   if(charge_state.charge_percent == 100){
  //     text_layer_set_text(s_battery_layer, "\ue604");
  //   }else if(charge_state.charge_percent > 50){
  //     text_layer_set_text(s_battery_layer, "\ue601");
  //   }else if(charge_state.charge_percent == 50){
  //     text_layer_set_text(s_battery_layer, "\ue603");
  //   }else{
  //     text_layer_set_text(s_battery_layer, "\ue602");
  //   }
  // }
  text_layer_set_text(s_battery_layer, "\ue606");
}

static void handle_bluetooth(bool connected) {
  text_layer_set_text(s_connection_layer, connected ? "\ue606" : "\ue605");
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%l:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);

  // Display the battery layer
  handle_battery(battery_state_service_peek());

  // Display the bluetooth layer
  handle_bluetooth(bluetooth_connection_service_peek());
}

static void main_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 100, 142, 30));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");

  //Apply to TextLayer
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // Create the battery layer
  s_battery_layer = text_layer_create(GRect(0, -2, 142, 25));
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, s_system_font);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text(s_battery_layer, "\ue604");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));

  // Connection layer
  s_system_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SYSTEM_18));
  s_connection_layer = text_layer_create(GRect(2, -2, 142, 25));
  text_layer_set_text_color(s_connection_layer, GColorBlack);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, s_system_font);
  text_layer_set_text_alignment(s_connection_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_connection_layer));

  //Invert the colors of the screen
  //s_inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  //layer_add_child(window_get_root_layer(window), inverter_layer_get_layer(s_inverter_layer));
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  // Unload system font
  fonts_unload_custom_font(s_system_font);

  // Destroy TextLayer
  text_layer_destroy(s_time_layer);

  // Destroy TextLayer
  text_layer_destroy(s_battery_layer);

  // Destroy Bluetooth layer
  text_layer_destroy(s_connection_layer);

  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);

  //Destroy the inverter
  //inverter_layer_destroy(s_inverter_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

  // Register with Battery Service
  battery_state_service_subscribe(&handle_battery);

  // Register with bluetooth service
  bluetooth_connection_service_subscribe(&handle_bluetooth);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
