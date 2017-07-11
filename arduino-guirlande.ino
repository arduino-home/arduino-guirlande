#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <EEPROM.h>

//needed for library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager

#include <ArduinoJson.h> // https://github.com/bblanchon/ArduinoJson

template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; }
static const char * endl PROGMEM = "\n";

#define CONFIG_PIN D1

#define RED_PIN D5
#define GREEN_PIN D6
#define BLUE_PIN D7

#define CONFIG_ADDRESS 0

struct config_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;

  inline void reset() {
    r = 0;
    g = 0;
    b = 0;
  }
  
  inline void load() {
    EEPROM.begin(sizeof(*this));
    Serial << "config load" << endl;
    EEPROM.get(CONFIG_ADDRESS, *this);
    Serial << "config loaded red=" << r << ", green=" << g << ", blue=" << b << endl;
  }

  inline void save() {
    Serial << "config save red=" << r << ", green=" << g << ", blue=" << b << endl;
    EEPROM.put(CONFIG_ADDRESS, *this);
    EEPROM.commit();
    Serial << "config saved" << endl;
  }
};

class RGBServer {
  ESP8266WebServer server;
  config_t & config;
  
public:

  RGBServer(config_t & pconfig)
   : server(80), 
     config(pconfig) {

    server.on("/status", HTTP_GET, [this](){
      server.send(200, "application/json", "{ \"r\": " + String(config.r) + ", \"g\": " + String(config.g) + ", \"b\": " + String(config.b) + " }");
    });
    
    server.on("/status", HTTP_POST, [this](){
      StaticJsonBuffer<200> buffer;
      JsonObject& data = buffer.parseObject(server.arg("plain"));

      if(!data.success()) {
        server.send(400);
        return;
      }
      
      config.r = data["r"];
      config.g = data["g"];
      config.b = data["b"];
      
      setRGB();
      config.save();

      server.send(200);
    });

    server.begin();
  }

  void loop() {
    server.handleClient();
  }
};

static config_t config;
static std::unique_ptr<RGBServer> server;

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
}

inline bool isConfigRequested() {
  return digitalRead(CONFIG_PIN) == LOW;
}

inline void setRGB() {
  Serial << "SetRGB red=" << config.r << ", green=" << config.g << ", blue=" << config.b << endl;
  
  analogWrite(RED_PIN, config.r);
  analogWrite(GREEN_PIN, config.g);
  analogWrite(BLUE_PIN, config.b);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n Starting");

  pinMode(CONFIG_PIN, INPUT_PULLUP);

  if(isConfigRequested()) {
    configWifi();
    config.reset();
    config.save();
    ESP.reset();
    return;
  }

  config.load();
  setRGB();
  WiFi.begin();

  server = std::unique_ptr<RGBServer>(new RGBServer(config));
}

void loop() {
  debugWifiStatus();

  server->loop();
}

