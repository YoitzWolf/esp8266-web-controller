#ifndef STREAM_HH
#define STREAM_HH

#ifndef ESPASYNC
    #include <ESP8266WebServerSecure.h>
#else
    #include <ESPAsyncWebserver.h>
#endif

#include <string>
#include <deque>
#include <map>

class StreamConnection {
    public:
    IPAddress client_ip;
    BearSSL::WiFiClientSecure client;
};

class StreamEvent {
    private:
    unsigned long _time_stamp;
    std::string tag;
    std::string msg;

    public:
    StreamEvent(
        unsigned long _time_stamp,
        std::string tag,
        std::string msg
    ):
        _time_stamp(_time_stamp),
        tag(tag),
        msg(msg)
    {};
    unsigned long time_stamp(){
        return this->_time_stamp;
    };
    std::string format();
};

/**
 * Used for broadcast at the moment
 */
/*
template<typename ChannelId, typename StreamConnection>
class OutputStream {
    private:
    std::deque<StreamEvent> stream_events;
    std::map<ChannelId, StreamConnection> connections;

    public:

    void cleanup();

    void push();

    void add_message(StreamEvent);

    ChannelId add_connection(StreamConnection);

    unsigned int connections_size() {
        return this->connections.size();
    }
    unsigned int messages_size() {
        return this->connections.size();
    }

    OutputStream(
        std::deque<StreamEvent> stream_events,
        std::map<ChannelId, StreamConnection> connections
    ):
        stream_events(stream_events),
        connections(connections)
    {};
};*/

#endif