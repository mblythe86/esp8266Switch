#include <FS.h>
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <EEPROM.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
//#include <ESP8266mDNS.h>
#include <JC_Button.h> 
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson
//#include <PubSubClient.h>
#include <SoftwareSerial.h>

#define BUTTON_PIN 13
#define RELAY_PIN  15
#define LED_PIN     2

#define PULLUP true
#define INVERT true
#define DEBOUNCE_MS 100

Button myBtn(BUTTON_PIN, PULLUP, INVERT, DEBOUNCE_MS);    //Declare the button

#define OFF 1
#define ON 0


//MDNSResponder mdns;
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
WiFiManager wifi;
WiFiClient espClient;
//PubSubClient mqttClient(espClient);
SoftwareSerial tx0;
//SoftwareSerial tx1; in use - TX
//SoftwareSerial tx2; in use - LED
//SoftwareSerial tx3; in use - RX
SoftwareSerial tx4;
SoftwareSerial tx5;
//SoftwareSerial tx6;
//SoftwareSerial tx7;
//SoftwareSerial tx8;
//SoftwareSerial tx9;
//SoftwareSerial tx10;
//SoftwareSerial tx11;
SoftwareSerial tx12;
//SoftwareSerial tx13; in use - button
SoftwareSerial tx14;
//SoftwareSerial tx15; in use - relay
SoftwareSerial tx16;

//define your default values here, if there are different values in config.json, they are overwritten.
char device_name[40] = "espSwitch";
char wifi_hostname[64];
//char mqtt_topic[120];
//char mqtt_server[40];
//char mqtt_port[6] = "1883";

int state = OFF;

//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback ()
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}


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

void handleNotFound()
{
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ )
  {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
}

void handle_root()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/html",index_html);
}

void handle_post()
{
  if (server.args() == 3)
  {
    Serial.print("Name: ");
    Serial.println(server.arg(0));
    Serial.print("MQTT IP: ");
    Serial.println(server.arg(1));
    Serial.print("MQTT Port: ");
    Serial.println(server.arg(2));

    Serial.println("saving config");
    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["device_name"] = server.arg(0);
    jsonDoc["mqtt_server"] = server.arg(1);
    jsonDoc["mqtt_port"] = server.arg(2);

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
      Serial.println("failed to open config file for writing");
    }

    serializeJsonPretty(jsonDoc, Serial);
    serializeJson(jsonDoc, configFile);
    configFile.close();
  }
  else
  {
    Serial.println("Error");
  }
  server.send(200, "text/plain", "ok");
}

void handle_on()
{
  Serial.println("State: ON");
  state = ON;
}

void handle_off()
{
  Serial.println("State: OFF");
  state = OFF;
}

//void mqttCallback(char* topic, byte* payload, unsigned int length)
//{
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");
//  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
//  }
//  Serial.println();
//
//  //ON
//  if ((char)payload[1] == 'N')
//  {
//    handle_on();
//  }
//  else
//  {
//    handle_off();
//  }
//}

long lastReconnect = 0;

/*
void mqttReconnect()
{
  long now = millis();
  if (now - lastReconnect > 5000)
  {
    lastReconnect = now;
    Serial.print("Attempting MQTT connection as:");
    Serial.println(device_name);
    if (mqttClient.connect(device_name))
    {
      Serial.println("connected");
      mqttClient.subscribe(mqtt_topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}
*/

void setup()
{
  Serial.begin(115200);
  tx0.begin(115200, SWSERIAL_8N1, -1, 0);
  tx4.begin(115200, SWSERIAL_8N1, -1, 4);
  tx5.begin(115200, SWSERIAL_8N1, -1, 5);
  tx12.begin(115200, SWSERIAL_8N1, -1, 12);
  tx14.begin(115200, SWSERIAL_8N1, -1, 14);
  tx16.begin(115200, SWSERIAL_8N1, -1, 16);

  pinMode(BUTTON_PIN, INPUT);

  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin())
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json"))
    {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile)
      {
        Serial.println("opened config file");
        size_t size = configFile.size();
        DynamicJsonDocument jsonDoc(size);
        auto error = deserializeJson(jsonDoc,configFile);
        //JsonObject& json = jsonDoc.parseObject(buf.get());
        serializeJsonPretty(jsonDoc,Serial);
        //json.printTo(Serial);
        if (!error)
        {
          Serial.println("\nparsed json");
          strcpy(device_name, jsonDoc["device_name"]);
          //strcpy(mqtt_server, jsonDoc["mqtt_server"]);
          //strcpy(mqtt_port, jsonDoc["mqtt_port"]);
        }
        else
        {
          Serial.print("failed to load json config: ");
          Serial.println(error.c_str());
        }
      }
    }
  }
  else
  {
    Serial.println("failed to mount FS");
  }

  WiFiManagerParameter custom_device_name("devicename", "device name", device_name, 40);
  WiFiManagerParameter custom_mqtt_server("server", "mqtt server", "" /*mqtt_server*/, 40);
  WiFiManagerParameter custom_mqtt_port("port", "mqtt port", "" /*mqtt_port*/, 5);

   //set config save notify callback
  wifi.setSaveConfigCallback(saveConfigCallback);
  
  //add all your parameters here
  wifi.addParameter(&custom_device_name);
  wifi.addParameter(&custom_mqtt_server);
  wifi.addParameter(&custom_mqtt_port);

  //read updated parameters
  strcpy(device_name, custom_device_name.getValue());
  //strcpy(mqtt_server, custom_mqtt_server.getValue());
  //strcpy(mqtt_port, custom_mqtt_port.getValue());

  sprintf(wifi_hostname, "espSwitch-%06x", ESP.getChipId());
  wifi.setHostname(wifi_hostname);
  wifi.autoConnect(device_name);
  
  //save the custom parameters to FS
  if (shouldSaveConfig)
  {
    Serial.println("saving config");
    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["device_name"] = device_name;
    //jsonDoc["mqtt_server"] = mqtt_server;
    //jsonDoc["mqtt_port"] = mqtt_port;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
      Serial.println("failed to open config file for writing");
    }

    serializeJsonPretty(jsonDoc,Serial);
    serializeJson(jsonDoc,configFile);
    configFile.close();
  }
  Serial.print("mDNS Name: ");
  Serial.println(device_name);

/*
  if (!mdns.begin(device_name, WiFi.localIP()))
  {
    Serial.println("Error setting up MDNS responder!");
  }
  else
  {
    Serial.println("mDNS responder started");
  }
  */

  // Start TCP (HTTP) server
  server.begin();
  Serial.println("TCP server started");
  Serial.print("Sketch size: ");
  Serial.println(ESP.getSketchSize());
  Serial.print("Free size: ");
  Serial.println(ESP.getFreeSketchSpace());
  
  server.on("/", HTTP_GET, handle_root);
  server.on("/", HTTP_POST, handle_post);
  server.on("/on", handle_on);
  server.on("/off", handle_off);
  server.onNotFound ( handleNotFound );

  httpUpdater.setup(&server);

  state = OFF;
  Serial.println("State: OFF");
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.print("local ip:");
  Serial.println(WiFi.localIP());

  //Serial.print("mqtt server Name: ");
  //Serial.println(mqtt_server);
  //mqttClient.setServer(mqtt_server, 1883);
  //mqttClient.setCallback(mqttCallback);
  //sprintf(mqtt_topic, "/%s/switch", device_name);

  setup_ota_updates();
}

void loop()
{
  ota_loop();
  
  //if (!mqttClient.connected())
  //{
  //  mqttReconnect();
  //}

  server.handleClient();
  //mqttClient.loop();
  myBtn.read();

  if (myBtn.wasPressed())
  {
    if (state == ON)
    {
      Serial.println("State: OFF");
      //mqttClient.publish(mqtt_topic, "OFF");
      state = OFF;
    }
    else
    {
      Serial.println("State: ON");
      //mqttClient.publish(mqtt_topic, "ON");
      state = ON;
    }
  }
  if (myBtn.pressedFor(10000))
  {
    Serial.println("Factory Reset");
    SPIFFS.format();
    wifi.resetSettings();
    ESP.restart();
  }
  digitalWrite(RELAY_PIN, state);

  if(wifi.getLastConxResult() == 3){ // WL_CONNECTED
    digitalWrite(LED_PIN, ON);
  }
  else{
    digitalWrite(LED_PIN,OFF);
  }
  tx0.println("GPIO 0");
  tx4.println("GPIO 4");
  tx5.println("GPIO 5");
  tx12.println("GPIO 12");
  tx14.println("GPIO 14");
  tx16.println("GPIO 16");
}

