#ifndef CONST_HPP
#define CONST_HPP
#define ESP8266 1
#include <Arduino.h>

#define ESP_LED 2
#define MCU_LED 16

#define DHTPIN 0
#define DHTTYPE DHT11

#define PORT 443

#define CACHE_SIZE 5
#define USE_CACHE

// UNIX EPOCH TIME, DO NOT USE FOR DELAYS
#define SECOND 1L
#define MINUTE 60L
#define HOUR 3600L
const unsigned long DEFAULT_TIMER = 10*SECOND;

// in millis got by ESP platform inside timeClient
const unsigned long TIME_UPDATE_INTERVAL = 10000;
// 10800 is MSK (in seconds)
const unsigned long TIMEZONE_GAP = 10800;

static const char HTML_CLI_VIEW[] PROGMEM = R"EOF(
    <!DOCTYPE html><html>
    <body>
    <input id="command" style="width:100%;max-width:100%;"/>
    <hr/>
    <textarea id="console" readonly="true" style="width:100%; max-width:100%;">
    </textarea>
    <script>
        let console_area = document.getElementById("console");
        console_area.value = "";
        
        function getSseData() {
                var evtSource = new EventSource('/cli/stream',
                  {
                    withCredentials: true
                  }
                );
                console.log("Event Stream created");
                evtSource.onopen = function() {
                    console.log("Connection to server opened.");
                };
                evtSource.onmessage = function(e) {
                    console.log('EventSource.onmessage log');
                };
                evtSource.onerror = function() {
                    console.log("EventSource failed.");
                };
                evtSource.addEventListener("esp8266", function(e) {
                    if (e.data != "keep-alive") {
                        console.log(e);
                        console_area.value += e.data + "\n";
                    }
                }, false);
        };
        getSseData();
    </script>
    </body></html>
)EOF";

#endif