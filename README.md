

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


# DSL

```h
<environment mark> <cmd> <opts> 
```

- Each environment can have its own commands;
- Commands are case-independent, ```H``` and ```h``` are equal;
- It is recommended to configure short names for marks, commands and options;
- ```<env> H``` command is help and produce generated info about environment;
- ```<env> <cmd> H``` command is help and produce info about command and options;

## DSL Example
```h
// Relay[ID:01] signal ON.
R01 S ON

// Relay[ID:01] signal OFF.
R01 S OFF

// Relay[ID:01] run with switch off timer(15s).
R01 RT T 15s

// Relay[ID:01] run timer(15s) with periodic(every 24h) and period start delay of 0s.
R01 RT P 24h T 15s D 0s

// Relay[ID:01] drop presets and stop
R01 D
```