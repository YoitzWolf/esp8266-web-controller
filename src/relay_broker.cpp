#include "relay_broker.hpp"
#include <Ticker.h>
#include "utils.hpp"

CommandResult SetSignalCommand::run(InputCommand input, String &output){
    if (this->broker->is_borrowed()) {
        return CommandResult::Internal;
    }
    if (input.options.size() != 1) {
        return CommandResult::InvalidArgument;
    }
    auto s = input.options.front();
    if (s == "ON") {
        this->set_signal(this->on);
        return CommandResult::Done;
    } else if (s == "OFF") {
        this->set_signal(this->off);
        return CommandResult::Done;
    } else {
        return CommandResult::InvalidArgument;
    }
}

String SetSignalCommand::help() {
    return "Usage: ON or OFF option";
}


CommandResult RunTimerCommand::run(InputCommand input, String &output) {
    if (this->broker->is_borrowed()) {
        return CommandResult::Internal;
    }
    float timer  = -1;
    float period = -1;
    float delay  = 0;
    for(auto i=input.options.begin(); i!=input.options.end();i++) {
        if (*i == "T") {
            i++;
            if (i == input.options.end()) {
                return CommandResult::InvalidArgument;
            }
            timer = com_string_to_seconds(*i);
        } else if (*i == "D") {
            i++;
            if (i == input.options.end()) {
                return CommandResult::InvalidArgument;
            }
            delay = com_string_to_seconds(*i);
        } else if (*i == "P") {
            i++;
            if (i == input.options.end()) {
                return CommandResult::InvalidArgument;
            }
            period = com_string_to_seconds(*i);
        } else {
            return CommandResult::InvalidArgument;
        }
    }
    if (timer == -1) {
        return CommandResult::InvalidArgument;
    }
    Serial.println(">>>");
    Serial.print(timer);
    Serial.print(" ");
    Serial.print(period);
    Serial.print(" ");
    Serial.print(delay);
    Serial.println(">>>");
    std::function<void (uint8_t)> setter = this->set_signal;
    auto offs = this->off;
    auto ons = this->on;
    if (delay <= 0) {
        // no delay
        if (period < 0) {
            this->set_signal(this->on);
            this->ticker.once(
                timer,
                [setter, offs](){setter(offs);}  
            );
        } else {
            this->broker->borrow(this->name());
            auto *t2 = &this->ticker_2;
            this->ticker.attach(
                period,
                [setter, offs, ons, timer, t2](){
                    Serial.println("Period Run..");
                    setter(ons);
                    t2->once(
                        timer,
                        [setter, offs](){setter(offs);}  
                    );
                }  
            );
        }
    } else {
        //delay
        Ticker* t = &(this->ticker);
        auto* broker = this->broker;
        auto name = this->name();
        Serial.println("Delay found..");
        auto *t2 = &this->ticker_2;
        this->ticker.once(
            delay,
            [t, t2, period, timer, setter, offs, ons, name, broker]() {
                Serial.println("Startup after delay..");
                if (period < 0) {
                    Serial.println("Single Run..");
                    setter(ons);
                    t->once(
                        timer,
                        [setter, offs](){setter(offs);}  
                    );
                } else {
                    broker->borrow(name);
                    t->attach(
                        period,
                        [setter, t2, offs, ons, timer](){
                            Serial.println("Period Run..");
                            setter(ons);
                            t2->once(
                                timer,
                                [setter, offs](){setter(offs);}  
                            );
                        }  
                    );
                }
            }
        );
    }
    return Done;
}

String RunTimerCommand::help() {
    return "Usage with option pairs:\ntimer T <time>,\nstartup period P <time>,\nfirst start delay D<time>,\nIf P is not used timer is not repeated";
}

bool RunTimerCommand::stop() {
    this->ticker.detach();
    this->ticker_2.detach();
    this->set_signal(off);
    return true;
}