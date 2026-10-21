#include "appstate.hpp"
#include <LittleFS.h>
#include <string_view>

/*void Application::send_stream() {
    // this->cleanup();
    if (this->stream.connections_size() == 0 || this->stream.messages_size() == 0){
        return; // Nothing to do
    }
    this->stream.push();
    // Serial.println("Pushed");
}*/

String Application::poll_connection(ClientInfo connection) {
    auto it = this->connections.find(connection);
    Serial.println(this->connections.size());
    Serial.println(connection.client_ip);
    // Serial.println(connection.client_port);
    if (it == this->connections.end()) {
        this->connections.insert(
            {
                connection,
                PollDescription()
            }
        );
        it = this->connections.find(connection);
    }
    auto epoch = this->get_ntc_timer()->getEpochTime();
    it->second.last_request_epoch = epoch;
    String json = "{ \"data\": [ ";
    unsigned long last = 0;
    for(auto evt: this->stream_events) {
        if (evt.time_stamp() > it->second.last_seen_epoch) {
            last = evt.time_stamp();
            std::string fmt = evt.format();
            size_t index = 0;
            while (true) {
                index = fmt.find("\"", index);
                if (index == std::string::npos) break;
                fmt = fmt.substr(0, index) + "\\" + fmt.substr(index, fmt.size());
                index++;
            }
            json.concat( ("\"" + fmt + "\",").c_str() );
        }
        yield();
    }
    it->second.last_seen_epoch = last;
    json.setCharAt(json.length()-1, ' ');
    json.concat("]}");
    Serial.println("Done..");
    return json;
}

void Application::clean_dead(){
    auto epoch = this->get_ntc_timer()->getEpochTime();
    for(auto it=this->connections.begin(); it!=this->connections.end();) {
        if (it->second.last_request_epoch + this->keep_alive_secs < epoch) {
            Serial.println("Client Dead..");
            it = this->connections.erase(it);
        } else {
            it++;
        }
    }
}

void Application::add_timer(
    AppTimerType timer_type,
    AppTimerCreationFlag creation_flag,
    std::string timer_name,
    unsigned long time_in_ms,
    std::function<void(void)> run,
    bool repeat
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
    switch (creation_flag)
    {
        case AppTimerCreationFlag::VIRTUAL_WAIT :
        case AppTimerCreationFlag::VIRTUAL_HANDLE_NOW: {
            /* PASS */
            break;
        }
        case AppTimerCreationFlag::MEMO_SYNC : {
            unsigned long delta = 0;
            unsigned long  time_stamp = this->get_ntc_timer()->getEpochTime();
            if (LittleFS.exists(timer_name.c_str())) {
                Serial.println("READ OLD DATA");
                auto file = LittleFS.open(timer_name.c_str(), "r");
                unsigned long base_epoch;
                base_epoch = file.parseInt();
                if (base_epoch + time_in_ms/1000 > time_stamp) {
                    delta = base_epoch + time_in_ms/1000 - time_stamp;
                }
                file.close();
                Serial.println("Read file success!");
            } else {
                Serial.println("CREATE EMPTY");
                Serial.println("Load memo timer file created!");
                auto file = LittleFS.open(timer_name.c_str(), "w");
                file.write(0);
                file.close();
            }
            auto temp_name = timer_name + "-MEMO-TIME-TMP";
            callback = [timer_name, callback, this, temp_name, time_in_ms]() {
                auto call_time = this->get_ntc_timer()->getEpochTime();
                this->flag_timer_callbacks.insert(
                    {
                        temp_name,
                        [timer_name, callback, this, temp_name, time_in_ms, call_time](){
                            auto file = LittleFS.open(timer_name.c_str(), "w");
                            file.write(call_time);
                            file.close();
                            yield();
                            this->flag_timer_callbacks.erase(temp_name);
                        }
                    }
                );
                callback();
                this->flag_timer_bus.insert(temp_name);
            };
            auto old_time = time_in_ms;
            auto old_timer_name = timer_name;
            auto old_callback = callback;
            this->add_timer(
                FLAG_CALL,
                VIRTUAL_WAIT,
                old_timer_name,
                old_time,
                old_callback,
                repeat
            );
            callback = [old_timer_name, this, old_callback]() {
                Serial.println("Temp Event Call!");
                this->try_handle_timer(old_timer_name);
                unsigned long  time_stamp = this->get_ntc_timer()->getEpochTime();
                this->add_message(
                    StreamEvent(
                        time_stamp,
                        "Sys",
                        "Temp Event Call!"
                    )
                );
                old_callback();
            };
            timer_name += "-TMP";
            repeat = false;
            timer_type = AppTimerType::FLAG_CALL;
            time_in_ms = std::max((unsigned long) 201, delta * 1000);
            creation_flag = AppTimerCreationFlag::VIRTUAL_HANDLE_NOW;
            break;
        }       
    }
    this->timers.insert(
        {
            timer_name,
            TimerState(
                time_in_ms,
                callback,
                this->get_ntc_timer(),
                repeat
            )
        }
    );
    if (creation_flag == AppTimerCreationFlag::VIRTUAL_HANDLE_NOW) {
        this->try_handle_timer(timer_name);
    }
}

void Application::handle_flag_timers() {
    for(auto timer_flag : this->flag_timer_bus) {
        yield();
        if (this->flag_timer_callbacks.find(timer_flag) != this->flag_timer_callbacks.end()) {
            this->flag_timer_callbacks[timer_flag]();
        } else {
            Serial.println("Not found");
        }
        auto tim = this->get_timer(timer_flag);
        if (tim!=nullptr) {
            // clean single-run timers
            if (!tim->is_repeat()) {
                this->flag_timer_callbacks.erase(timer_flag);
                this->timers.erase(timer_flag);
            }
        } else {
            Serial.println(("Ghost timer calls found: " + timer_flag).c_str());
        }
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