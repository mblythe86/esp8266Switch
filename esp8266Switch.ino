#include <JC_Button.h>

#include "function_declarations.h"

#define BUTTON_PIN 13
#define RELAY_PIN 15
#define LED_PIN 2

#define PULLUP true
#define INVERT true
#define DEBOUNCE_MS 100

Button myBtn(BUTTON_PIN, PULLUP, INVERT, DEBOUNCE_MS);  //Declare the button

int state = OFF;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT);

  load_config();

  setup_wifi();

  maybe_save_config();  //If changed on the captive portal

  http_setup();

  state = OFF;
  Serial.println("State: OFF");
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  setup_mqtt();

  setup_ota_updates();
}

void turn_off() {
  Serial.println("State: OFF");
  mqtt_publish("OFF");
  state = OFF;
  digitalWrite(RELAY_PIN, state);
}

void turn_on() {
  Serial.println("State: ON");
  mqtt_publish("ON");
  state = ON;
  digitalWrite(RELAY_PIN, state);
}

void loop() {
  wifi_loop(LED_PIN);

  ota_loop();

  http_loop();

  mqtt_loop();

  myBtn.read();

  if (myBtn.wasPressed()) {
    if (state == ON) {
      turn_off();
    } else {
      turn_on();
    }
  }
  if (myBtn.pressedFor(10000)) {
    Serial.println("Factory Reset");
    reset_config();
    reset_wifi();
    ESP.restart();
  }
}
