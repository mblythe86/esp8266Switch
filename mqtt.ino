#include <WiFiClient.h>
#include <PubSubClient.h>

#include "function_declarations.h"

WiFiClient espClient;
PubSubClient mqttClient(espClient);

char mqtt_topic[120];

extern char device_name[64];
extern char mqtt_server[64];
extern char mqtt_port[6];

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //ON
  if ((char)payload[1] == 'N') {
    handle_on();
  } else {
    handle_off();
  }
}

long lastReconnect = 0;

void mqttReconnect() {
  long now = millis();
  if (now - lastReconnect > 5000) {
    lastReconnect = now;
    Serial.print("Attempting MQTT connection as:");
    Serial.println(device_name);
    if (mqttClient.connect(device_name)) {
      Serial.println("connected");
      mqttClient.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}

void setup_mqtt() {
  Serial.print("mqtt server Name: ");
  Serial.println(mqtt_server);
  mqttClient.setServer(mqtt_server, 1883);  //FIXME: use mqtt_port
  mqttClient.setCallback(mqttCallback);
  sprintf(mqtt_topic, "/%s/switch", device_name);
}

void mqtt_loop() {
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  if (mqttClient.connected()) {
    mqttClient.loop();
  }
}

void mqtt_publish(String state) {
  if (mqttClient.connected()) {
    mqttClient.publish(mqtt_topic, state.c_str());
  }
}