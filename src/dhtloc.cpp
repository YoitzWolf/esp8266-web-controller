
#include "dhtloc.hpp"

DHTResult read_dht(DHT dht) {
    float h = dht.readHumidity();
    // Read temperature as Celsius
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit
    //float f = dht.readTemperature(true);
    return DHTResult(t, h);
}