#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson
#include <ArduinoHomeLibrary.h>   // https://github.com/arduino-home/arduino-home-library

#define CONFIG_PIN  D1
#define RED_PIN     D5
#define GREEN_PIN   D6
#define BLUE_PIN    D7

void setup() {
  AH_DEBUG_INIT();

  Runtime::setName("arduino-guirlande");

  Runtime::registerService(new DispatcherService());
  Runtime::registerService(new ConfigurationService());
  Runtime::registerService(new WifiSetupService(CONFIG_PIN));
  Runtime::registerService(new HttpService());
  Runtime::registerService(new InfoService());
  Runtime::registerService(new RGBService(RED_PIN, GREEN_PIN, BLUE_PIN));

  Runtime::setup();
}

void loop() {
  Runtime::loop();
}

