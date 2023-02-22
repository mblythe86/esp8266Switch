#include <ESP8266WiFi.h>  //https://github.com/esp8266/Arduino
#include <WiFiManager.h>  //https://github.com/tzapu/WiFiManager

#include "function_declarations.h"

WiFiManager wifi;

extern char device_name[64];
extern char mqtt_server[64];
extern char mqtt_port[6];

void setup_wifi() {
  WiFiManagerParameter custom_device_name("devicename", "device name", device_name, 40);
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 5);

  //set config save notify callback
  wifi.setSaveConfigCallback(saveConfigCallback);

  //add all your parameters here
  wifi.addParameter(&custom_device_name);
  wifi.addParameter(&custom_mqtt_server);
  wifi.addParameter(&custom_mqtt_port);

  wifi.autoConnect(device_name);

  //read updated parameters
  strcpy(device_name, custom_device_name.getValue());
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  strcpy(mqtt_port, custom_mqtt_port.getValue());

  Serial.print("local ip:");
  Serial.println(WiFi.localIP());
}

#define LED_OFF 1
#define LED_ON 0
void wifi_loop(uint pin) {
  if (wifi.getLastConxResult() == 3) {  // WL_CONNECTED
    digitalWrite(pin, LED_ON);
  } else {
    digitalWrite(pin, LED_OFF);
  }
}

void reset_wifi() {
  wifi.resetSettings();
}