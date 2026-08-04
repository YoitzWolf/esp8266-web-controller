#include "appstate.hpp"

void Application::load_command(
    unsigned long time_stamp,
    std::string tag,
    std::string cmd
) {
    this->stream.add_message(
        StreamEvent(
            time_stamp,
            "CMD:" + tag,
            cmd
        )
    );
    // parse any cmds or solve any access question
    //  unknown command example
    //  this->user_cmds.push_back(
    //     StreamEvent(
    //         time_stamp,
    //         tag + " Error",
    //         "Unkown Command"
    //     )
    // );
    //  responce example
    //  this->user_cmds.push_back(
    //     StreamEvent(
    //         time_stamp,
    //         tag + " Responce",
    //         "Command output here"
    //     )
    // );
}

void Application::send_stream() {
    // this->cleanup();
    if (this->stream.connections_size() == 0 || this->stream.messages_size() == 0){
        return; // Nothing to do
    }
    this->stream.push();
    // Serial.println("Pushed");
}

void Application::add_timer(
    AppTimerType timer_type,
    std::string timer_name,
    unsigned long time_in_ms,
    std::function<void(void)> run,
    bool repeat,
    bool run_now
) {
    std::function<void(void)> callback = nullptr;
    switch (timer_type) {
        case AppTimerType::FLAG_CALL : {
            this->flag_timer_callbacks.insert(
                {timer_name, run}
            );
            auto* bus_ptr = &this->flag_timer_bus;
            callback = [bus_ptr, timer_name]() {
                bus_ptr->insert(timer_name);
            };
            break;
        }
        case AppTimerType::FUNC_CALL : {
            callback = run;
            break;
        }
    }
    this->timers.insert(
        {
            timer_name,
            TimerState(
                time_in_ms,
                callback,
                this->ntc_client,
                repeat
            )
        }
    );
    if (run_now) {
        this->try_handle_timer(timer_name);
    }
}

void Application::handle_flag_timers() {
    for(auto timer_flag : this->flag_timer_bus) {
        this->flag_timer_callbacks[timer_flag]();
    }
    this->flag_timer_bus.clear();
}

bool Application::try_handle_timer(
    std::string timer_name
) {
    auto timer = this->get_timer(timer_name);
    if (timer) {
        timer->handle_timer();
        return true;
    } else {
        return false;
    }
}

TimerState* Application::get_timer(
    std::string timer_name
) {
    auto x = this->timers.find(timer_name);
    if (x == this->timers.end()) {
        return nullptr;
    } else {
        return &x->second;
    }
}