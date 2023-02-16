#include <ArduinoOTA.h>

//extern String ota_passwd;
//extern String wifi_hostname;
char ota_hostname[64];

void setup_ota_updates(){
  // Port defaults to 8266
  ArduinoOTA.setPort(8267);

  // Hostname defaults to esp8266-[ChipID]
  //char hostname[22];
  sprintf(ota_hostname, "espSwitch-%06x", ESP.getChipId());
  //wifi_hostname.toCharArray(ota_hostname,63);
  ArduinoOTA.setHostname(ota_hostname);

  // No authentication by default
  //Serial.println("Setting OTA password to " + ota_passwd);
  //ArduinoOTA.setPassword(ota_passwd.c_str());
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void ota_loop(){
  ArduinoOTA.handle();
}

