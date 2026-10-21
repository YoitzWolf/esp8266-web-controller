#ifndef CONST_HPP
#define CONST_HPP

#define ESP8266 1
#include <Arduino.h>
#include "passkeys.hpp"

#define ESP_LED 2
#define MCU_LED 16

#define RELAY 13
#define RELAY_ON pinMode(RELAY, INPUT)
#define RELAY_OFF pinMode(RELAY, OUTPUT)

#define DHTPIN 5
#define DHTTYPE DHT11

#define PORT 443

#define CACHE_SIZE 2
#define USE_CACHE

// UNIX EPOCH TIME, DO NOT USE FOR DELAYS
#define SECOND 1L
#define MINUTE 60L
#define HOUR 3600L
const unsigned long DEFAULT_TIMER = 5*SECOND;


const unsigned long MS = 1000;
// in millis got by ESP platform inside timeClient
const unsigned long TIME_UPDATE_INTERVAL = MINUTE*1000; // 1 minute
// 10800 is MSK (in seconds)
const unsigned long TIMEZONE_GAP = 10800;

#define LOCAL_ESP_DNS "esp8266"

static const char HTML_CLI_VIEW[] PROGMEM = R"EOF(
    <!DOCTYPE html><html>
    <header>
        <style>
            * {
                margin: 0;
                padding: 0;
            }
            
            hr {
                margin: 10px;
            }

            .c100 {
                width:100%;
                max-width:100%;
                min-width:100%;
            }
        </style>
    </header>
    <body style="padding:30px;">
    <input id="command" class="c100"/>
    <button id="command-run" class="c100" style="min-height:50px; background: lightgreen; " onclick="btnOnClick()"> Send Command </button>
    <hr/>
    <textarea id="console" class="c100" readonly="true" style="min-height:500px;">
    </textarea>
    <script>
        let console_area = document.getElementById("console");
        let command = document.getElementById("command");
        let on_poll = false;
        console_area.value = "";
        command.value = "help";
        function add_stream(stream) {
            let data = stream.data;
            console.log('Success:', data);
            data.forEach((item, i) => {
                console.log(i, item);
                console_area.value += item + "\n";
                console_area.scrollTop = console_area.scrollHeight;
            });
        }
        function btnOnClick() {
            on_poll=true;
            console.log(command.value);
            request = new Request(
                "/cli/poll", {
                    method: "POST",
                    credentials: 'include',
                    body: command.value,
                    headers: {
                        'Content-Type': 'text/plain'
                    }
                }
            );
            fetch(request)
                .then(response => response.json())
                .then(data => {add_stream(data); on_poll=false;})
                .catch(error => console.error('Error:', error));
        };
        function startPollTimer() {
            if (!on_poll) {
                let request = new Request(
                    "/cli/poll", {
                        method: "GET",
                        credentials: 'include',
                        headers: {
                            'Content-Type': 'text/plain'
                        }
                    }
                );
                fetch(request)
                    .then(response => response.json())
                    .then(data => add_stream(data))
                    .catch(error => console.error('Error:', error));
            };
        };
        setTimeout(
            () => {
                setInterval(startPollTimer, 15000);
            }, 2000
        );
    </script>
    </body></html>
)EOF";


#endif