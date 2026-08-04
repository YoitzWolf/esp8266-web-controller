
#include "stream.hpp"
#include "utils.hpp"

std::string StreamEvent::format() {
    return "[" + timestamp_to_str(this->time_stamp) + " | " +
        this->tag + "]: " + this->msg + ";";
}

void OutputStream::cleanup() {
    for(auto it: this->connections) {
        if (it.second.client.connected()) {
            it.second.client.println("event: esp8266");
            it.second.client.println("data: keep-alive");
            it.second.client.println();
            it.second.client.flush();
        } else {
            Serial.println("Client Dead..");
            it.second.client.flush();
            it.second.client.stop();
            this->connections.erase(it.first);
        }
    }
    if (this->connections.size() == 0) {
        this->stream_events.clear();
    }
}

void OutputStream::push() {
    this->cleanup();
    while (this->stream_events.size() != 0) {
        StreamEvent evt = this->stream_events.front();
        this->stream_events.pop_front();
        for(auto it: this->connections) {
            it.second.client.println("event: esp8266");
            it.second.client.println(("data: " + evt.format()).c_str() );
            it.second.client.println();
            it.second.client.flush();
        }
    }
}

void OutputStream::add_message(StreamEvent event) {
    if(this->connections_size() > 0) {
        this->stream_events.push_back(event);
    } // else there no reason to memorize
}

std::string OutputStream::add_connection(StreamConnection connection) {
    // auto last = this->connections.end();
    // unsigned long maxid = 0;
    // if (last != this->connections.begin()) {
    //     std::prev(last)->first + 1;
    // }
    std::string s = connection.client_ip.toString().c_str();
    this->connections.insert(
        {s, connection}
    );
    return s;
}