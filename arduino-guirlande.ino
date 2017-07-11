#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }
static const char * endl PROGMEM = "\n";

#define CONFIG_PIN D1

inline const char * wifiStatusToString() {
  switch(WiFi.status()) {
    case WL_CONNECTED: return "WL_CONNECTED";
    case WL_NO_SHIELD: return "WL_NO_SHIELD";
    case WL_IDLE_STATUS: return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL: return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED: return "WL_SCAN_COMPLETED";
    case WL_CONNECT_FAILED: return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED: return "WL_DISCONNECTED";
  }
  return "<unknown>";
}

inline void debugWifiStatus() {
  static int oldStatus = -1;

  if(oldStatus == WiFi.status()) { return; }
  oldStatus = WiFi.status();

  Serial << WiFi.SSID() << " " << wifiStatusToString() << endl;
}

inline void configWifi() {
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(true);

  String name = "Guirlande-" + String(ESP.getChipId());
  if (!wifiManager.startConfigPortal(name.c_str())) {
    Serial.println("failed to connect and hit timeout");
    ESP.reset();
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected");
  ESP.reset();
}

inline bool isConfigRequested() {
  return digitalRead(CONFIG_PIN) == LOW;
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n Starting");

  pinMode(CONFIG_PIN, INPUT_PULLUP);

  if(isConfigRequested()) {
    configWifi();
    return;
  }

  WiFi.begin();
}

void loop() {

  debugWifiStatus();
}

