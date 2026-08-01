
#ifndef DHTLOC_HPP
#define DHTLOC_HPP

#include <DHT.h>

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
};

DHTResult read_dht(DHT &dht);


#endif