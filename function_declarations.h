//esp8266Switch.ino
#define OFF 0
#define ON 1
void turn_off();
void turn_on();

//OTA.ino
void setup_ota_updates();
void ota_loop();

//http.ino
void http_setup();
void http_loop();

//config.ino
void saveConfigCallback();
void load_config();
void maybe_save_config();
void store_config_values(String _device_name, String _mqtt_server, String _mqtt_port);
void reset_config();

//wifi.ino
void setup_wifi();
void wifi_loop(uint pin);
void reset_wifi();

//mqtt.ino
void setup_mqtt();
void mqtt_loop();
void mqtt_publish(String state);

//dps310.ino
void dps310_setup();
void dps310_loop();