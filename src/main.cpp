#include "constants.hpp"

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266mDNS.h>

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Ticker.h>

#include "dhtloc.hpp"
#include "timer.hpp"
#include "utils.hpp"
#include "appstate.hpp"
#include "fs_utils.hpp"
#include "command.hpp"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

BearSSL::ESP8266WebServerSecure server(PORT);
BearSSL::ServerSessions serverCache(CACHE_SIZE);

DHT dht(DHTPIN, DHTTYPE);
CommandServer* com_server;
std::shared_ptr<Application> app;
std::weak_ptr<Application> weak_app;

TimerCmdConfig* timer_cmd_conf;


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
    RELAY_OFF;
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
    // com_server = new CommandServer();
    app = std::shared_ptr<Application>(
        new Application(
            &timeClient,
            &dht
        )
    );
    weak_app = app;
    app->add_timer(
        FLAG_CALL,
        VIRTUAL_HANDLE_NOW,
        "NTC-update-timer",
        TIME_UPDATE_INTERVAL,
        [](){
            auto tim = app->get_ntc_timer();
            tim -> forceUpdate();
            auto timeStamp = tim->getEpochTime();
            Serial.println("Time Synced");
            app->add_message(
            StreamEvent(
                timeStamp,
                "Sys",
                "Sync time: " + timestamp_to_str(timeStamp)));
        },
        true
    );
    
    /*app->add_timer(
        FLAG_CALL,
        MEMO_SYNC,
        "Long Memo Timer",
        MINUTE*1000, // to milliseconds
        []()
        {   
            auto timeStamp = app->get_ntc_timer()->getEpochTime();
            digitalWrite(MCU_LED, LOW);
            app->add_message(
                StreamEvent(
                    timeStamp,
                    "Sys",
                    "Memo-sync timer event"
                )
            );
            digitalWrite(MCU_LED, HIGH);
        },
        true
    );*/

    app->add_timer(
        FLAG_CALL,
        VIRTUAL_WAIT, //MEMO_SYNC,
        "Relay-On",
        24*60*MINUTE*MS,
        [](){
            // auto tim = app->get_ntc_timer();
            // tim -> forceUpdate();
            // auto timeStamp = tim->getEpochTime();
            // app->add_message(
            // StreamEvent(
            //     timeStamp,
            //     "Sys",
            //     "Sync time: " + timestamp_to_str(timeStamp)));
            RELAY_ON;
            auto timeStamp = app->get_ntc_timer()->getEpochTime();
            digitalWrite(MCU_LED, LOW);
            app->add_message(
                StreamEvent(
                    timeStamp,
                    "Sys",
                    "Relay-on"
                )
            );
            Serial.println("Relay timer log: Relay on");
            app->add_timer(
                FLAG_CALL,
                VIRTUAL_HANDLE_NOW,
                "Relay-off",
                10*SECOND*MS,
                [](){
                    RELAY_OFF;
                    auto timeStamp = app->get_ntc_timer()->getEpochTime();
                    Serial.println("Relay timer log: Relay off");
                    app->add_message(
                        StreamEvent(
                            timeStamp,
                            "Sys",
                            "Relay-off"
                        )
                    );
                },
                false
            );
        },
        true
    );
    /*-------------------------------*/
    com_server = new CommandServer();
    timer_cmd_conf = new TimerCmdConfig (
        weak_app
    );
    com_server->register_cmd (
      timer_cmd_conf  
    );
    /*-------------------------------*/
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
        });
    server.on(
        "/cli/poll",
        []()
        {
            digitalWrite(ESP_LED, LOW);
            Serial.printf("Free RAM: %d bytes\n", ESP.getFreeHeap());
            if (!auth(server))
            {
                return server.send(403, "text/plain", "Denied");
            }
            auto client = server.client();
            Serial.println("Poll");
            String cmd = server.arg("plain");
            cmd.trim();
            Serial.println(cmd);
            if (auto shared = weak_app.lock()) {
                if (cmd.length() > 0) {
                    std::string ipStr  = client.remoteIP().toString().c_str();
                    std::string cmdStr = cmd.c_str(); 
                    app->add_message(
                        StreamEvent (
                            app->get_ntc_timer()->getEpochTime(),
                            ipStr,
                            cmdStr
                        )
                    );
                    CommandString tokens = {};
                    //cmd.indexOf
                    int it = cmd.indexOf(' ');
                    int it_last = 0;
                    if (it == -1) {
                        tokens.push_back(cmdStr);
                    } else {
                        while (it != -1) {
                            if (it != 0) {
                                std::string sub = cmd.substring(it_last, it).c_str();
                                tokens.push_back( sub );
                            }
                            // cmd = cmd.substring(it+1, cmd.length());
                            it_last = it+1;
                            it = cmd.indexOf(' ', it+1);
                        }
                        std::string sub = cmd.substring(it_last, cmd.length()).c_str();
                        tokens.push_back( sub );
                    }
                    Serial.println("Tokenized");
                    auto com = com_server->parse(tokens);
                    if (com != std::nullopt) {
                        Serial.print("Run");
                        auto res = com.value()->run();
                        Serial.print(res.second.c_str());
                        app->add_message(
                            StreamEvent (
                                app->get_ntc_timer()->getEpochTime(),
                                "Sys",
                                res.second
                            )
                        );
                    } else {
                        Serial.println("Invalid");
                        app->add_message(
                        StreamEvent (
                                app->get_ntc_timer()->getEpochTime(),
                                "Sys",
                                "Invalid command"
                            )
                        ); 
                    }
                }
                String json = shared->poll_connection( {client.remoteIP(), client.remotePort()} );
                shared.reset();
                digitalWrite(ESP_LED, HIGH);
                server.send(200, "text/json", json);
                json.clear();
            } else {
                server.send(512, "text/plain", "Ptr error");
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

// String formattedDate;
unsigned long timeStamp;

void loop()
{
    MDNS.update();
    server.handleClient();
    yield();
    app->handle_flag_timers();
    yield();
    app->clean_dead();
    yield();
}

/*server.on(
      "/cli/post",
      HTTP_POST,
      []() {
        digitalWrite(ESP_LED, LOW);
        BearSSL::WiFiClientSecure client = server.client();
        IPAddress clientIP = server.client().remoteIP();
        Serial.println("CLI POST>>");
        Serial.println("Input wait");
        // while(!client.available()){}
        Serial.println(client.available());

        if (!auth(server)) {
          server.send(403, "text/plain", "ACCESS DENIED");
          return;
        }
        String cmd = server.arg("plain");
        Serial.println(cmd);
        Serial.println("--");
        if (cmd.length() != 0) {
            app->add_message(
                StreamEvent (
                    app->get_ntc_timer()->getEpochTime(),
                    clientIP.toString().c_str(),
                    cmd.c_str()
                )
            );
            server.send(200, "text/plain", "PASSED");
        } else {
            server.send(200, "text/plain", "EMPTY_REQUEST");
        }
    });*/