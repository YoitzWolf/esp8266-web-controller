#include "command.hpp"
#include <Arduino.h>
#include "utils.hpp"

std::optional<Command*> CommandServer::parse(CommandString tokens) {
    if (tokens.size() == 0) {
        Serial.println("Empty command");
        return std::nullopt; 
    }
    auto s = this->com_register.find(tokens[0]);
    if (s == this->com_register.end()) { return std::nullopt; }
    Serial.println("Command found..");
    yield();
    return s->second->parse(tokens);
};

void CommandServer::register_cmd(CommandConfig* cfg) {
    this->com_register.insert(
        {cfg->name(), cfg}
    );
}

std::vector<std::string> CommandServer::cmd_list() {
    std::vector<std::string> v = {};
    for(auto [com_name, com]: this->com_register) {
        v.push_back(com_name);
    }
    return v;
}

std::optional<Command*>
TimerCmdConfig::parse(CommandString tokens) {
    Serial.println("Tokens input:");
    Serial.println(tokens.size());
    if (tokens.size() == 0) { return std::nullopt; }
    // for (unsigned int i=0; i<tokens.size();) {
    //     auto cmd_view = tokens[i]; //std::string_view(tokens[i]);
    //     cmd_view = cmd_view.erase(0, cmd_view.find_first_not_of(" \n\r\t"));
    //     yield();
    //     cmd_view = cmd_view.erase(cmd_view.find_last_not_of(" \n\r\t")+1);
    //     yield();
    //     tokens[i] = cmd_view;
    // }
    for (auto i=tokens.begin(); i!=tokens.end();) {
        Serial.println(i->c_str());
        yield();
        if (i->size() > 0) {
            i++;
        } else {
            i = tokens.erase(i);
        }
    }
    Serial.println("Cleaned, tokens amount:");
    Serial.println(tokens.size());
    if (tokens[0] != TimerCmdConfig::NAME ) { return std::nullopt; }
    //if (auto shared_app = this->app.lock()) {
        switch (tokens.size()) {
            case 1: {
                // only timer command found
                auto weak = this->app;
                return new Command(
                    [weak]() {
                        if (auto shared_app = weak.lock()) {
                            std::string s = "timers: ";
                            for(auto it: shared_app->get_timers()) {
                                s += it + " ";
                            }
                            return CommandResult{
                                CommandResultType::Ok,
                                s
                            };
                            shared_app.reset();
                        } else {
                            return CommandResult{
                                CommandResultType::InternalError,
                                "app cannot be locked"
                            };
                        }
                    }
                );
            }
            case 2: {
                auto weak = this->app;
                auto timer_name = tokens[1];
                Serial.println(("("+ timer_name + ")").c_str());
                return new Command(
                    [weak, timer_name]() {
                        if (auto shared_app = weak.lock()) {
                            auto timer = shared_app->get_timer(timer_name);
                            if (timer == nullptr) {
                                return CommandResult{
                                    CommandResultType::Ok,
                                    "Timer not found"
                                };
                            } else {
                                String s = String(timer->get_last_tick());
                                Serial.println("");
                                s.concat(String ("last tick"));
                                Serial.println(s);
                                auto last_tick  = timestamp_to_str(timer->get_last_tick());
                                auto timer_tick = timestamp_to_str_time(timer->period_ms()/1000);
                                std::string res = timer_name + ": last-tick=" + last_tick + " period=" + timer_tick;
                                return CommandResult{
                                    CommandResultType::Ok,
                                    res
                                };
                            }
                            shared_app.reset();
                        } else {
                            return CommandResult{
                                CommandResultType::InternalError,
                                "app cannot be locked"
                            };
                        }
                    }
                );
            }
            case 3 ... 128: {
                auto weak = this->app;
                auto timer_name = tokens[1];
                auto command    = tokens[2];
                return new Command(
                    [weak, timer_name, command, tokens]() {
                        if (auto shared_app = weak.lock()) {
                            auto timer = shared_app->get_timer(timer_name);
                            if (timer == nullptr) {
                                return CommandResult{
                                    CommandResultType::Ok,
                                    "Timer not found"
                                };
                            } else {
                                if (command == "re-run") {
                                    timer->handle_timer();
                                    return CommandResult{
                                        CommandResultType::Ok,
                                        "done"
                                    };  
                                }
                                else if (command == "fast-call") {
                                    timer->fast_call();
                                    return CommandResult{
                                        CommandResultType::Ok,
                                        "done"
                                    };  
                                }
                                else if (command == "set") {
                                    return CommandResult{
                                        CommandResultType::InternalError,
                                        "not implemented"
                                    };  
                                } else {
                                    return CommandResult{
                                        CommandResultType::UnknownArgument,
                                        "comand cannot be interpreted"
                                    };  
                                }
                            }
                            shared_app.reset();
                        } else {
                            return CommandResult{
                                CommandResultType::InternalError,
                                "app cannot be locked"
                            };
                        }
                    }
                );
            }
            default: {
                return std::nullopt;
            }
        }
    // } else {
    //    return std::nullopt;
    //}*/
    return std::nullopt;
}