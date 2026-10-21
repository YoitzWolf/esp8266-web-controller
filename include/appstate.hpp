#ifndef APPSTATE_HH
#define APPSTATE_HH

#include "constants.hpp"
#include "stream.hpp"
#include "timer.hpp"
#include <map>
#include <deque>
#include <set>
#include <string>
#include <DHT.h>
#include "dhtloc.hpp"

class ClientInfo {
    public:
    IPAddress client_ip;
    uint16_t client_port;

    bool operator<(const ClientInfo &o) const {
        auto a1= this->client_ip.v4();
        auto a2= o.client_ip.v4();
        return a1 < a2; // || (a1==a2 && this->client_port < o.client_port);
    }

    bool operator==(const ClientInfo &o) const {
        auto a1= this->client_ip.v4();
        auto a2= o.client_ip.v4();
        return (a1==a2);// && this->client_port == o.client_port);
    }
};

class PollDescription {
    public:
    unsigned long last_seen_epoch=0;
    unsigned long last_request_epoch=0;
};


enum AppTimerType {
    FLAG_CALL,
    FUNC_CALL
};

enum AppTimerCreationFlag {
    VIRTUAL_WAIT,
    VIRTUAL_HANDLE_NOW,
    MEMO_SYNC,
};

class Application{
    private:
    // OutputStream<ClientInfo, PollDescription> stream = OutputStream<ClientInfo, PollDescription>(
    //     {},
    //     {}
    // );
    std::deque<StreamEvent> stream_events={};
    std::map<ClientInfo, PollDescription> connections;
    
    protected:
    unsigned long max_mem = 16;
    unsigned long keep_alive_secs = 60;
    NTPClient* ntc_client;
    std::map<std::string, TimerState> timers;
    std::map<std::string, std::function<void(void)>> flag_timer_callbacks;
    std::set<std::string> flag_timer_bus;
    #ifdef DHTTYPE
    DHT* dht_ptr;
    #endif

    public:
    Application(
        NTPClient* ntc_client,
        DHT* dht_ptr
    ):
        ntc_client(ntc_client),
        dht_ptr(dht_ptr),
        stream_events(std::deque<StreamEvent>()),
        connections({})
    { };

    DHT* get_dht() {
        return this->dht_ptr;
    }

    NTPClient* get_ntc_timer() {
        return this->ntc_client;
    }

    /*void load_command(
        unsigned long time_stamp,
        CommandServer* com_server,
        std::string tag,
        std::string cmd
    );*/

    void add_timer(
        AppTimerType timer_type,
        AppTimerCreationFlag creation_flag,
        std::string timer_name,
        unsigned long time_in_ms,
        std::function<void(void)> run,
        bool repeat=false
    );

    bool try_handle_timer(
        std::string timer_name
    );

    void handle_flag_timers();

    TimerState* get_timer(
        std::string timer_name
    );

    void add_message(StreamEvent evt) {
        this->stream_events.push_back(
            evt
        );
        while (this->stream_events.size() > this->max_mem) {
            this->stream_events.pop_front();
        }
    }

    String poll_connection(ClientInfo);

    void clean_dead();
    void cleanup() {
        //this->stream.cleanup();
        this->stream_events.clear();
    }

    std::vector<std::string> get_timers() {
        std::vector<std::string> v = {};
        for(auto &[key, val] : this->timers) {
            v.push_back(key);
        }
        return v;
    }

};

#endif