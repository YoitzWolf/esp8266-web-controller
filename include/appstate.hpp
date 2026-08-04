#ifndef APPSTATE_HH
#define APPSTATE_HH

#include "constants.hpp"
#include "stream.hpp"
#include "timer.hpp"
#include <map>
#include <set>
#include <string>
#include <DHT.h>

enum AppTimerType {
    FLAG_CALL,
    FUNC_CALL
};

class Application{
    private:
    OutputStream stream = OutputStream(
        {},
        {}
    );
    
    protected:
    NTPClient* ntc_client=nullptr;
    std::map<std::string, TimerState> timers;
    std::map<std::string, std::function<void(void)>> flag_timer_callbacks;
    std::set<std::string> flag_timer_bus;
    #ifdef DHTTYPE
    DHT* dht_ptr = nullptr;
    #endif

    // std::deque<StreamEvent> user_cmds;

    public:
    Application(){};

    Application(
        NTPClient* ntc_client,
        DHT* dht_ptr = nullptr
    ):
        ntc_client(ntc_client) 
        #ifdef DHTTYPE
        ,dht_ptr(dht_ptr)
        #endif
    {
    }

    DHT* get_dht() {
        return this->dht_ptr;
    }

    NTPClient* get_ntc_timer() {
        return this->ntc_client;
    }

    void load_command(
        unsigned long time_stamp,
        std::string tag,
        std::string cmd
    );

    void add_timer(
        AppTimerType timer_type,
        std::string timer_name,
        unsigned long time_in_ms,
        std::function<void(void)> run,
        bool repeat=false,
        bool run_now=false
    );

    bool try_handle_timer(
        std::string timer_name
    );

    void handle_flag_timers();

    TimerState* get_timer(
        std::string timer_name
    );

    void send_stream();

    void add_message(StreamEvent evt) {
        this->stream.add_message(evt);
    }

    void add_connection(StreamConnection con) {
        this->stream.add_connection(con);
    }

    void cleanup() {
        this->stream.cleanup();
    }

};

#endif