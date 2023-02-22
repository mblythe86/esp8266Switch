#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// This example shows how to read temperature/pressure

#include <Adafruit_DPS310.h>

extern char device_name[64];

Adafruit_DPS310 dps;

// Can also use SPI!
#define DPS310_CS D0
uint dps310_configured = 0;

void dps310_setup() {
  Serial.println("Init DPS310");
  if (!dps.begin_I2C()) {
    Serial.println("Failed to init DPS");
  }
  else{
    Serial.println("DPS OK!");
    dps310_configured = 1;
    dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
    dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
  }
}

float temperature_sum = 0;
float pressure_sum = 0;
uint count = 0;
void dps310_loop() {
  if(!dps310_configured){
    return;
  }
  if (!dps.temperatureAvailable() || !dps.pressureAvailable()) {
    return;  // wait until there's something to read
  }

  sensors_event_t temp_event, pressure_event;
  dps.getEvents(&temp_event, &pressure_event);
  temperature_sum += temp_event.temperature;
  pressure_sum += pressure_event.pressure;

  Serial.print(F("Temperature = "));
  Serial.print(temp_event.temperature);
  Serial.println(" *C");
  Serial.print(F("Temperature = "));
  Serial.print(temp_event.temperature * 9.0 / 5.0 + 32);
  Serial.println(" *F");
  Serial.print(F("Pressure = "));
  Serial.print(pressure_event.pressure);
  Serial.println(" hPa");
  Serial.print(F("Count = "));
  Serial.println(++count);
  Serial.println();

  if (count == 64) {
    // Prepare JSON document
    DynamicJsonDocument doc(2048);
    doc["name"] = device_name;
    doc["temperature_c"] = String(temperature_sum / count);
    doc["pressure_hPa"] = String(pressure_sum / count);

    // Serialize JSON document
    String json;
    serializeJson(doc, json);
    Serial.println(json);

    WiFiClient client;  // or WiFiClientSecure for HTTPS
    HTTPClient http; //FIXME: use ESP8266AsyncHttpClient to get this out of the loop?

    // Send request
    //FIXME: switch hard-coded URL to a config value
    http.begin(client, "http://192.168.42.3:9292/temp_pres/save");
    int httpCode = http.POST(json);

    // Read response
    Serial.print("HTTP response code: ");
    Serial.println(httpCode);
    if (httpCode == HTTP_CODE_OK) {
      Serial.println(http.getString());
    } else {
      Serial.println(http.errorToString(httpCode));
    }

    // Disconnect
    http.end();

    count = 0;
    temperature_sum = 0;
    pressure_sum = 0;
  }
}
