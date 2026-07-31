#ifndef CONST_HPP
#define CONST_HPP

#define ESP8266 1

#define ESP_LED 2
#define MCU_LED 16

#define PORT 443

#define CACHE_SIZE 5
#define USE_CACHE

// UNIX EPOCH TIME, DO NOT USE FOR DELAYS
#define SECOND 1L
#define MINUTE 60L
#define HOUR 3600L
const unsigned long DEFAULT_TIMER = 10*SECOND;

// in millis got by ESP platform inside timeClient
const unsigned long TIME_UPDATE_INTERVAL = 500;
// 10800 is MSK (in seconds)
const unsigned long TIMEZONE_GAP = 10800;
#endif