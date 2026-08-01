#include "constants.hpp"
#include "passkeys.hpp"

#include <Arduino.h>
#include <ESP8266WiFi.h>
// #include <BearSSL::WiFiServerSecure::ClientTypeSecure.h>
#include <ESP8266WebServerSecure.h>


#include <NTPClient.h>
#include <WiFiUdp.h>

#include "dhtloc.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "appstate.hpp"

TimerState app_timer_state( 0, DEFAULT_TIMER ); 

const char* ssid = NET_ID;
const char* password = NET_PWD;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

BearSSL::ESP8266WebServerSecure server(PORT);
BearSSL::ServerSessions serverCache(CACHE_SIZE);

DHT dht(DHTPIN, DHTTYPE);

Application *app;

bool auth(BearSSL::ESP8266WebServerSecure &server){
  return server.authenticate("admin", "admin123");
}



void serverSentEventHeader(BearSSL::WiFiClientSecure client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/event-stream;");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  client.println("Access-Control-Allow-Origin: *");  // allow any connection. We don't want Arduino to host all of the website ;-)
  client.println("Cache-Control: no-cache");  // refresh the page automatically every 5 sec
  client.println();
  client.flush();
}
 
void serverSentEvent(BearSSL::WiFiClientSecure client) {
  client.println("event: esp8266"); // this name could be anything, really.
  client.println("data: keep-alive");
  client.println();
  client.flush();
}

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
  app = new Application();
  server.on("/",
    HTTP_GET,
    []() {
      digitalWrite(ESP_LED, LOW);
      if (!auth(server)) {
        return server.requestAuthentication();
      }
      digitalWrite(ESP_LED, HIGH);
      server.send(200, "text/plain", "Logged");
  });
  server.on(
    "/cli/view",
    HTTP_GET,
    []() {
      digitalWrite(ESP_LED, LOW);
      if (!auth(server)) {
        return server.requestAuthentication();
      }
      digitalWrite(ESP_LED, HIGH);
      server.send(
        200,
        "text/html",
        HTML_CLI_VIEW
      );
  });
  server.on(
    "/cli/stream",
    []() {
      digitalWrite(ESP_LED, LOW);
      if (!auth(server)) {
        return server.send(403, "text/plain", "Denied");
      }
      auto client = server.client();
      Serial.println("new Client");
      serverSentEventHeader(client);
      serverSentEvent(client);
      Serial.println(client.status());
      Serial.println(client.available());
      app->add_connection(
        {
          client.remoteIP(), client
        }
      );
      // while (client.connected()) {
      //   digitalWrite(ESP_LED, LOW);
      //   serverSentEvent(client);
      //   delay(10000);
      // }
      // delay(16);
      // client.stop();
      // Serial.println("Client dead");
      digitalWrite(ESP_LED, HIGH);
      // server.sendContent_P(PSTR("HTTP/1.1 200 OK\nContent-Type: text/event-stream;\nConnection: keep-alive\nCache-Control: no-cache\nAccess-Control-Allow-Origin: *\n\n"));
      
  });
  /*server.on(
    "/cli/post",
    HTTP_POST,
    []() {
      digitalWrite(ESP_LED, LOW);
      if (!auth(server)) {
        server.send(403, "text/plain", "ACCESS DENIED");
        return;
      }
      // pass comand
      digitalWrite(ESP_LED, HIGH);
      IPAddress clientIP = server.client().remoteIP(); 

      server.send(200, "text/plain", "PASSED");
  });*/
  server.begin();
  digitalWrite(ESP_LED, HIGH);
  digitalWrite(MCU_LED, HIGH);
  app_timer_state.set_last_tick(timeClient.getEpochTime());
  dht.begin();
}

// String formattedDate;
unsigned long timeStamp;

void loop() {
  Serial.println("LOOP { ");
  digitalWrite(ESP_LED, HIGH);
  while(!timeClient.update()) {
    // timeClient.forceUpdate();
    server.handleClient();
    // Serial.print(">>");
    timeStamp = timeClient.getEpochTime();
    bool timer_res = app_timer_state.handle_timer(timeStamp);
    if (timer_res) {
      app->add_message (
        StreamEvent (
          timeStamp,
          "Sys",
          "Timer runned"
        )
      );
    }
    app->send_stream();
  }
  digitalWrite(ESP_LED, LOW);
  // digitalWrite(ESP_LED, HIGH);
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  timeStamp = timeClient.getEpochTime();
  // Extract date
  // int splitT = formattedDate.indexOf("T");
  // timeStamp = formattedDate.substring(0, splitT);
  Serial.print("TIMESTAMP SYNCED: ");
  Serial.println(timeStamp);
  Serial.print(">>");
  Serial.print(timestamp_to_str(timeStamp).c_str());
  Serial.println("");
  // delay(100);
  app->add_message(
    StreamEvent(
      timeStamp,
      "Sys",
      "Sync time: " + timestamp_to_str(timeStamp)
    )
  );
  app->send_stream();
  Serial.println("};");
}

