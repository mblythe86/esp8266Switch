#include <ESP8266WebServer.h>

#include "function_declarations.h"

extern int state;

ESP8266WebServer server(80);

const char* index_html =
  R"(<html><body><form method='POST' action='/' enctype='multipart/form-data'>
                  <fieldset>
                    <legend>General:</legend>
                    Switch Name: <input type='text' name='switchname'>
                  </fieldset>
                  <fieldset>
                    <legend>MQTT:</legend>
                    MQTT Server: <input type='text' name='mqttserver'>
                    MQTT Port: <input type='text' name='mqttport' value='1883'>
                    <input type='submit' value='Submit'>
                  </fieldset>
               </form></body></html>)";

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

void handle_root() {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html", index_html);
}

void handle_post() {
  if (server.args() == 3) {
    store_config_values(server.arg(0), server.arg(1), server.arg(2));
  } else {
    Serial.println("Error");
  }
  server.send(200, "text/plain", "ok");
}

void handle_on() {
  turn_on();
  server.send(200, "text/plain", "ok");
}

void handle_off() {
  turn_off();
  server.send(200, "text/plain", "ok");
}

void handle_status() {
  if(state == ON){
    server.send(200, "text/plain", "on");
  }
  else if(state == OFF){
    server.send(200, "text/plain", "off");
  }
  else{
    server.send(200, "text/plain", "unknown");
  }
}

void http_setup() {
  // Start TCP (HTTP) server
  server.begin();
  Serial.println("TCP server started");

  server.on("/", HTTP_GET, handle_root);
  server.on("/", HTTP_POST, handle_post);
  server.on("/on", handle_on);
  server.on("/off", handle_off);
  server.on("/status", handle_status);
  server.on("/state", handle_status);
  server.onNotFound(handleNotFound);
}

void http_loop() {
  server.handleClient();
}