#ifndef FSUTILS_HPP
#define FSUTILS_HPP

#include <LittleFS.h>

void fs_begin() {
    LittleFSConfig cfg;
    // cfg.setAutoFormat(false);
    LittleFS.setConfig(cfg);
    Serial.println(LittleFS.begin());
}



#endif