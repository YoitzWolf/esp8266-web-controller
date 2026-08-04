
#ifndef DHTLOC_HPP
#define DHTLOC_HPP

#include <DHT.h>

#include <string>

class DHTResult{
    public:
    float temperature;
    float humidity;

    DHTResult(
        float temperature,
        float humidity
    ): 
        temperature(temperature),
        humidity(humidity)
    {};

    std::string to_json() {
        char bufstring[64];
        std::sprintf(
            bufstring,
            "{\"humidity\": \"%03.1f\"; \"temperature\": \"%03.1f\";}", humidity, temperature);
        std::string sf = bufstring;
        return sf;
    }
};

DHTResult read_dht(DHT *dht);


#endif