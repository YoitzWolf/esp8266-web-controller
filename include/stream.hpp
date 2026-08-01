#ifndef STREAM_HH
#define STREAM_HH

#include <ESP8266WebServerSecure.h>
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
    unsigned long time_stamp;
    std::string tag;
    std::string msg;

    public:
    StreamEvent(
        unsigned long time_stamp,
        std::string tag,
        std::string msg
    ):
        time_stamp(time_stamp),
        tag(tag),
        msg(msg)
    {};

    std::string format();
};

/**
 * Used for broadcast at the moment
 */
class OutputStream {
    private:
    std::deque<StreamEvent> stream_events;
    std::map<std::string, StreamConnection> connections;

    public:

    void cleanup();

    void push();

    void add_message(StreamEvent);

    std::string add_connection(StreamConnection);

    unsigned int connections_size() {
        return this->connections.size();
    }
    unsigned int messages_size() {
        return this->connections.size();
    }

    OutputStream(
        std::deque<StreamEvent> stream_events,
        std::map<std::string, StreamConnection> connections
    ):
        stream_events(stream_events),
        connections(connections)
    {};
};

#endif