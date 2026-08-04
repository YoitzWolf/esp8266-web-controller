#include "constants.hpp"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServerSecure.h>

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>

#include "dhtloc.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "appstate.hpp"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

BearSSL::ESP8266WebServerSecure server(PORT);
BearSSL::ServerSessions serverCache(CACHE_SIZE);

DHT dht(DHTPIN, DHTTYPE);

Application *app;

bool auth(BearSSL::ESP8266WebServerSecure &server)
{
    return server.authenticate("admin", "admin123");
}

void serverSentEventHeader(BearSSL::WiFiClientSecure client)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/event-stream;");
    client.println("Connection: close");              // the connection will be closed after completion of the response
    client.println("Access-Control-Allow-Origin: *"); // allow any connection. We don't want Arduino to host all of the website ;-)
    client.println("Cache-Control: no-cache");        // refresh the page automatically every 5 sec
    client.println();
    client.flush();
}

void serverSentEvent(BearSSL::WiFiClientSecure client)
{
    client.println("event: esp8266"); // this name could be anything, really.
    client.println("data: keep-alive");
    client.println();
    client.flush();
}

void setup()
{
    Serial.begin(9600);
    pinMode(ESP_LED, OUTPUT);
    pinMode(MCU_LED, OUTPUT);
    pinMode(DHTPIN, INPUT);
    dht.begin();

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(NET_ID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(NET_ID, NET_PWD);
    while (WiFi.status() != WL_CONNECTED)
    {
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
    /*-------------------------------*/
    Serial.print("SERVER PORT: ");
    Serial.println(PORT);
    Serial.println("");
    server.getServer().setRSACert(
        new BearSSL::X509List(server_cert),
        new BearSSL::PrivateKey(server_private_key));
    /*-------------------------------*/
    timeClient.forceUpdate();
    app = new Application(
        &timeClient,
        &dht
    );
    app->add_timer(
        FLAG_CALL,
        "NTC update timer",
        TIME_UPDATE_INTERVAL,
        [](){
            auto tim = app->get_ntc_timer();
            tim -> forceUpdate();
            auto timeStamp = tim->getEpochTime();
            app->add_message(
            StreamEvent(
                timeStamp,
                "Sys",
                "Sync time: " + timestamp_to_str(timeStamp)));
        },
        true,
        true
    );
    app->add_timer(
        FLAG_CALL,
        "DHT Timer",
        DEFAULT_TIMER*1000, // to milliseconds
        []()
        {   
            digitalWrite(MCU_LED, LOW);
            DHT* dht = app->get_dht();
            dht->begin();
            auto dhtres = read_dht(dht);
            auto timeStamp = app->get_ntc_timer()->getEpochTime();
            app->add_message(
                StreamEvent(
                    timeStamp,
                    "Sys",
                    "DHT callback: " + dhtres.to_json()
                )
            );
            digitalWrite(MCU_LED, HIGH);
        },
        true,
        true);
    /*-------------------------------*/
    server.on("/",
              HTTP_GET,
              []()
              {
                  digitalWrite(ESP_LED, LOW);
                  if (!auth(server))
                  {
                      return server.requestAuthentication();
                  }
                  digitalWrite(ESP_LED, HIGH);
                  server.send(200, "text/plain", "Logged");
              });
    server.on(
        "/cli/view",
        HTTP_GET,
        []()
        {
            digitalWrite(ESP_LED, LOW);
            if (!auth(server))
            {
                return server.requestAuthentication();
            }
            digitalWrite(ESP_LED, HIGH);
            server.send(
                200,
                "text/html",
                HTML_CLI_VIEW);
        });
    server.on(
        "/cli/stream",
        []()
        {
            digitalWrite(ESP_LED, LOW);
            if (!auth(server))
            {
                return server.send(403, "text/plain", "Denied");
            }
            auto client = server.client();
            Serial.println("new Client");
            serverSentEventHeader(client);
            serverSentEvent(client);
            Serial.println(client.status());
            Serial.println(client.available());
            app->add_connection(
                {client.remoteIP(), client});
            digitalWrite(ESP_LED, HIGH);
            // server.sendContent_P(PSTR("HTTP/1.1 200 OK\nContent-Type: text/event-stream;\nConnection: keep-alive\nCache-Control: no-cache\nAccess-Control-Allow-Origin: *\n\n"));
        });
    server.on(
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
        BearSSL::WiFiClientSecure client = server.client();
        IPAddress clientIP = server.client().remoteIP();
        if (server.hasArg("cmd")) {
            app->load_command(
                timeClient.getEpochTime(),
                clientIP.toString().c_str(),
                server.arg("cmd").c_str()
            );
            server.send(200, "text/plain", "PASSED");
        } else {
            server.send(200, "text/plain", "EMPTY_REQUEST");
        }
    });
    server.begin();
    digitalWrite(ESP_LED, HIGH);
    digitalWrite(MCU_LED, HIGH);
    dht.begin();
    auto dhtres = read_dht(&dht);
    Serial.println(dhtres.to_json().c_str());
    // app_timer_state.set_last_tick(timeClient.getEpochTime());
    
}

// String formattedDate;
unsigned long timeStamp;

void loop()
{
    server.handleClient();
    app->handle_flag_timers();
    app->send_stream();

}
