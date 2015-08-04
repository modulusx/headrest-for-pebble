#include <pebble.h>

#define KEY_NAME 0
#define KEY_APP 1
#define KEY_CONNECTED 2
#define KEY_HEARD 3
#define KEY_LED 4
#define KEY_LED_ON 5
#define KEY_LED_OFF 6
#define KEY_NOTIFY 7

static Window *s_main_window;
static TextLayer *s_layer_name;
static TextLayer *s_layer_app;
static TextLayer *s_layer_heard;
static TextLayer *s_layer_connected;
bool ledState = true;

static void main_window_load(Window *window) {
  window_set_background_color(window, GColorBlack);
	
	s_layer_name = text_layer_create(GRect(0, 12, 144, 37));
  text_layer_set_background_color(s_layer_name, GColorBlack);
  text_layer_set_text_color(s_layer_name, GColorWhite);
  text_layer_set_text(s_layer_name, "Name: Loading...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_layer_name));

	s_layer_app = text_layer_create(GRect(0, 12 + 37, 144, 37));
  text_layer_set_background_color(s_layer_app, GColorBlack);
  text_layer_set_text_color(s_layer_app, GColorWhite);
  text_layer_set_text(s_layer_app, "App: Loading...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_layer_app));

	s_layer_connected = text_layer_create(GRect(0, 12 + 74, 144, 37));
  text_layer_set_background_color(s_layer_connected, GColorBlack);
  text_layer_set_text_color(s_layer_connected, GColorWhite);
  text_layer_set_text(s_layer_connected, "Connected: Loading...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_layer_connected));

	s_layer_heard = text_layer_create(GRect(0, 12 + 37 + 74, 144, 37));
  text_layer_set_background_color(s_layer_heard, GColorBlack);
  text_layer_set_text_color(s_layer_heard, GColorWhite);
  text_layer_set_text_alignment(s_layer_heard, GTextAlignmentCenter);
  text_layer_set_text(s_layer_heard, "Loading...");
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_layer_heard));
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_layer_name);
    text_layer_destroy(s_layer_app);
    text_layer_destroy(s_layer_connected);
    text_layer_destroy(s_layer_heard);
}

static void setLED(int newLEDState) {
  Tuplet value = TupletInteger(KEY_LED,newLEDState);
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_tuplet(iter,&value);
  app_message_outbox_send();
}

static void notifySpark(int sparkkey) {
  Tuplet value = TupletInteger(KEY_NOTIFY,sparkkey);
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  dict_write_tuplet(iter,&value);
  app_message_outbox_send();
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  setLED(KEY_LED_ON);
	ledState = true;
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  notifySpark(1);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  setLED(KEY_LED_OFF);
	ledState = false;
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
	window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char buffer_name[32];
  static char buffer_app[32];
  static char buffer_connected[8];
  static char buffer_heard[32];
	static char buffer_layer_name[32];
	static char buffer_layer_app[32];
	static char buffer_layer_connected[32];

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    switch(t->key) {
    case KEY_NAME:
      snprintf(buffer_name, sizeof(buffer_name), "%s", t->value->cstring);
      break;
    case KEY_APP:
      snprintf(buffer_app, sizeof(buffer_app), "%s", t->value->cstring);
      break;
    case KEY_CONNECTED:
      snprintf(buffer_connected, sizeof(buffer_connected), "%s", t->value->cstring);
      break;
    case KEY_HEARD:
      snprintf(buffer_heard, sizeof(buffer_heard), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }

  // Update text layeres with results
  snprintf(buffer_layer_name, sizeof(buffer_layer_name), "Name: %s", buffer_name);
  text_layer_set_text(s_layer_name, buffer_layer_name);
  snprintf(buffer_layer_app, sizeof(buffer_layer_app), "App: %s", buffer_app);
  text_layer_set_text(s_layer_app, buffer_layer_app);
  snprintf(buffer_layer_connected, sizeof(buffer_layer_connected), "Connected: %s", buffer_connected);
  text_layer_set_text(s_layer_connected, buffer_layer_connected);
  text_layer_set_text(s_layer_heard, buffer_heard);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  //APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

// Init the app
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
	
	// Set click config provider
	window_set_click_config_provider(s_main_window, click_config_provider);

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

// De-Init the app
static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

// Static portion of every pebble app given my current level of inexperience
int main(void) {
  init();
  app_event_loop();
  deinit();
}