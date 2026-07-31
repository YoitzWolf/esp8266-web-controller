

## ESP8266 secured webserver & timer


### include/passkeys.hpp example

```c++
// WIFI PWD NETWORK
#ifndef NETWORK_IDENTIFICATION
  #define NET_ID "..."
  #define NET_PWD "..."

// openssl req -x509 -nodes -newkey rsa:2048 -keyout key.pem -out cert.pem -days 4096
  const char server_cert[] = R"EOF(-----BEGIN CERTIFICATE-----
...
-----END CERTIFICATE-----
)EOF";
  const char server_private_key[] = R"EOF(-----BEGIN PRIVATE KEY-----
...
-----END PRIVATE KEY-----
)EOF";
#endif

```