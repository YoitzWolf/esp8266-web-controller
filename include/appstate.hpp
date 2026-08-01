#ifndef APPSTATE_HH
#define APPSTATE_HH

#include "stream.hpp"

class Application{
    private:
    OutputStream stream = OutputStream(
        {},
        {}
    );
    // std::deque<StreamEvent> user_cmds;

    public:
    Application() {
    }

    void load_command(
        unsigned long time_stamp,
        std::string tag,
        std::string cmd
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