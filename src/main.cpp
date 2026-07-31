#include "constants.hpp"
#include "passkeys.hpp"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#include <ESP8266WebServerSecure.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

class TimerState {
  private:
  unsigned long last_tick;
  unsigned long timer;
  std::function<void(void)> run=NULL;
  bool on_run=false;

  public:
  unsigned long get_last_tick() {
    return this->last_tick;
  }

  void set_last_tick(unsigned long val) {
    this->last_tick = val;
  }

  bool is_running() {
    return this->on_run;
  }

  void hadle_timer(unsigned long time_stamp) {
    auto delta = time_stamp - this->last_tick;
    if (delta >= this->timer) { // every 10 secs
      Serial.println("Run timer event");
      this->set_last_tick(time_stamp);
      this->on_run = true;
      this->run();
      this->on_run = false;
    }
  }

  TimerState(
    unsigned long last_tick,
    unsigned long timer,
    std::function<void(void)> run=[](){
      digitalWrite(MCU_LED, LOW);
      delay(2000); // 2 sec time work
      digitalWrite(MCU_LED, HIGH);
    }
  ):
    last_tick(last_tick), 
    timer(timer),
    on_run(false),
    run(run)
  { };
};

TimerState app_timer_state( 0, DEFAULT_TIMER ); 

const char* ssid = NET_ID;
const char* password = NET_PWD;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

BearSSL::ESP8266WebServerSecure server(PORT);
BearSSL::ServerSessions serverCache(CACHE_SIZE);

void setup() {
  Serial.begin(9600);
  pinMode(ESP_LED, OUTPUT);
  pinMode(MCU_LED, OUTPUT);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.setUpdateInterval(TIME_UPDATE_INTERVAL);
  timeClient.begin();
  timeClient.setTimeOffset(TIMEZONE_GAP);
  // server = AsyncWebServer(PORT);
  // events = AsyncEventSource("/events");
  Serial.print("SERVER PORT: ");
  Serial.println(PORT);
  Serial.println("");
  server.getServer().setRSACert(
    new BearSSL::X509List(server_cert),
    new BearSSL::PrivateKey(server_private_key)
  );
  server.on("/", []() {
    digitalWrite(ESP_LED, LOW);
    if (!server.authenticate("admin", "admin123")) {
      return server.requestAuthentication();
    }
    digitalWrite(ESP_LED, HIGH);
    server.send(200, "text/plain", "Login OK");
  });
  server.begin();
  digitalWrite(ESP_LED, HIGH);
  digitalWrite(MCU_LED, HIGH);
  app_timer_state.set_last_tick(timeClient.getEpochTime());
}

// String formattedDate;
unsigned long timeStamp;
unsigned long hours;
unsigned long minutes;
unsigned long seconds;

void loop() {
  Serial.println("LOOP { ");
  digitalWrite(ESP_LED, HIGH);
  while(!timeClient.update()) {
    timeClient.forceUpdate();
    server.handleClient();
    // Serial.print(">>");
    timeStamp = timeClient.getEpochTime();
    app_timer_state.hadle_timer(timeStamp);
  }
  digitalWrite(ESP_LED, LOW);
  // digitalWrite(ESP_LED, HIGH);
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  timeStamp = timeClient.getEpochTime();
  hours   = ((timeStamp % 86400L) / 3600);
  minutes = ((timeStamp % 3600L) / 60);
  seconds = ( timeStamp % 60);
  // Extract date
  // int splitT = formattedDate.indexOf("T");
  // timeStamp = formattedDate.substring(0, splitT);
  Serial.print("TIMESTAMP SYNCED: ");
  Serial.println(timeStamp);
  Serial.print(">>");
  Serial.print(hours);
  Serial.print(":");
  Serial.print(minutes);
  Serial.print(":");
  Serial.print(seconds);
  Serial.print(";");
  Serial.println("");
  delay(100);
  Serial.println("};");
}

