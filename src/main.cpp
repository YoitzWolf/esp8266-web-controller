#include "constants.hpp"

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266mDNS.h>

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>

#include "dhtloc.hpp"
#include "command.hpp"
#include "relay_broker.hpp"
#include "utils.hpp"
#include "fs_utils.hpp"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

BearSSL::ESP8266WebServerSecure server(PORT);
BearSSL::ServerSessions serverCache(CACHE_SIZE);

DHT dht(DHTPIN, DHTTYPE);


RelayBroker* relay_cmd_broker;

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
    pinMode(RELAY, OUTPUT);
    pinMode(DHTPIN, INPUT);
    digitalWrite(RELAY, RELAY_OFF);
    dht.begin();
    fs_begin();
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
    if (!MDNS.begin(LOCAL_ESP_DNS)) {
        Serial.println("Error setting up MDNS responder!");
        while (1) { delay(1000); }
    }
    Serial.println("mDNS::");
    Serial.println(LOCAL_ESP_DNS);
    MDNS.addService("https", "tcp", PORT);
    timeClient.setUpdateInterval(TIME_UPDATE_INTERVAL);
    timeClient.begin();
    timeClient.setTimeOffset(TIMEZONE_GAP);
    /*-------------------------------*/
    Serial.print("SERVER PORT: ");
    Serial.println(PORT);
    Serial.println("");
    server.getServer().setRSACert(
        new BearSSL::X509List(server_cert),
        new BearSSL::PrivateKey(server_private_key)
    );
    server.getServer().setCache(&serverCache);
    /*-------------------------------*/
    timeClient.forceUpdate();
    relay_cmd_broker = new RelayBroker(
        RELAY, RELAY_ON, RELAY_OFF, {}
    );
    server.on(
        "/",
        HTTP_GET,
        []() {
            digitalWrite(ESP_LED, LOW);
                  if (!auth(server))
                  {
                      return server.requestAuthentication();
                  }
                  digitalWrite(ESP_LED, HIGH);
                  server.send(200, "text/plain", "Logged");
        }         
    );
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
        }
    );
    server.on(
        "/cli/send",
        []()
        {
            digitalWrite(ESP_LED, LOW);
            Serial.printf("Free RAM: %d bytes\n", ESP.getFreeHeap());
            if (!auth(server))
            {
                return server.send(403, "text/plain", "Denied");
            }
            auto client = server.client();
            Serial.println("CMD RECIEVED:");
            String cmd = server.arg("plain");
            cmd.trim();
            cmd.toUpperCase();
            Serial.println(cmd);
            if (cmd.length() > 0) {
                auto f = cmd.indexOf(" ");
                auto name = cmd.substring(0, f);
                name.trim();
                cmd = cmd.substring(f);
                cmd.trim();
                String out = "";
                CommandResult res=CommandResult::InvalidArgument;
                if (name == "R01") {
                    res = relay_cmd_broker->interpretate(cmd, out);

                } else {
                    out = "Unknown environment";
                }
                String json = "{ \"data\": [ \"";
                json.concat(comm_res_to_str(res));
                json.concat(": ");
                json.concat(out);
                json.concat("\"]}");
                server.send(200, "text/json", json);

            }
            Serial.println("Request end");
            digitalWrite(ESP_LED, HIGH);
        });

    server.begin();
    digitalWrite(ESP_LED, HIGH);
    digitalWrite(MCU_LED, HIGH);
    auto dhtres = read_dht(&dht);
    Serial.println(dhtres.to_json().c_str());
}


void loop()
{
    MDNS.update();
    server.handleClient();
    yield();
    // app->handle_flag_timers();
    // yield();
    // app->clean_dead();
    // yield();
}