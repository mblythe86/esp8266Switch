#include <FS.h>
#include <EEPROM.h>
#include <ArduinoJson.h>  //https://github.com/bblanchon/ArduinoJson

#include "function_declarations.h"

//flag for saving data
bool shouldSaveConfig = false;

char device_name[64];
char mqtt_server[64];
char mqtt_port[6] = "1883";

//callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void load_config() {
  //read configuration from FS json
  Serial.println("mounting FS...");

  sprintf(device_name, "espSwitch-%06x", ESP.getChipId());
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        DynamicJsonDocument jsonDoc(size);
        auto error = deserializeJson(jsonDoc, configFile);
        //JsonObject& json = jsonDoc.parseObject(buf.get());
        serializeJsonPretty(jsonDoc, Serial);
        //json.printTo(Serial);
        if (!error) {
          Serial.println("\nparsed json");
          strcpy(device_name, jsonDoc["device_name"]);
          strcpy(mqtt_server, jsonDoc["mqtt_server"]);
          strcpy(mqtt_port, jsonDoc["mqtt_port"]);
        } else {
          Serial.print("failed to load json config: ");
          Serial.println(error.c_str());
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
}
void maybe_save_config() {
  if (shouldSaveConfig) {
    save_config();
  }
}

void save_config() {
  Serial.println("saving config");
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["device_name"] = device_name;
  jsonDoc["mqtt_server"] = mqtt_server;
  jsonDoc["mqtt_port"] = mqtt_port;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  serializeJsonPretty(jsonDoc, Serial);
  serializeJson(jsonDoc, configFile);
  configFile.close();
}

void store_config_values(String _device_name, String _mqtt_server, String _mqtt_port) {
  //FIXME: just set the globals from the strings, then call save_config()
  Serial.print("Name: ");
  Serial.println(_device_name);
  Serial.print("MQTT IP: ");
  Serial.println(_mqtt_server);
  Serial.print("MQTT Port: ");
  Serial.println(_mqtt_port);

  Serial.println("saving config");
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["device_name"] = _device_name;
  jsonDoc["mqtt_server"] = _mqtt_server;
  jsonDoc["mqtt_port"] = _mqtt_port;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }

  serializeJsonPretty(jsonDoc, Serial);
  serializeJson(jsonDoc, configFile);
  configFile.close();
}

void reset_config() {
  SPIFFS.format();  
}