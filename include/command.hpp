#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <functional>
#include <vector>
#include <string>
#include <map>
#include <optional>

#include <DHT.h>
#include "appstate.hpp"

using CommandString = std::vector<std::string> ;

enum CommandResultType {
    Ok,
    AccessDenied,
    InvalidUseage,
    UnknownArgument,
    InternalError
};

using CommandResult = std::pair<CommandResultType, std::string>;

using CommandCallback = std::function<CommandResult(void)>;

class Command {
    protected:
    CommandCallback callback = nullptr;
    public:
    Command(CommandCallback callback): callback(callback) {};
    CommandResult run() {
        return this->callback();
    }
};

class CommandConfig {
    protected:
        //CommandCallback callback;
        std::string cmd_name;
    public:
        CommandConfig(
            std::string cmd_name
        ): cmd_name(cmd_name) {};
        
        virtual std::optional<Command*> parse(CommandString) = 0;
        std::string name() { return this->cmd_name; };
};

template <typename A>
class AppCmdConfig : public CommandConfig {
    protected:
    std::weak_ptr<A> app;
    public:
    virtual std::optional<Command*> parse(CommandString) = 0;
    AppCmdConfig(
        std::string name,
        std::weak_ptr<A> app
    ): CommandConfig(name), app(app)
    {};
};

class TimerCmdConfig : public AppCmdConfig<Application> {
    public:
    inline static const std::string NAME = "timer";

    std::optional<Command*> parse(CommandString) override;
    // using AppCmdConfig<Application>::AppCmdConfig;
    TimerCmdConfig(
        std::weak_ptr<Application> app
    ): AppCmdConfig<Application>::AppCmdConfig(NAME, app) {};
};

class CommandServer {
    protected:
        std::map<std::string, CommandConfig*> com_register={};
    public:
        std::optional<Command*> parse(CommandString tokens);
        void register_cmd(CommandConfig*);
        std::vector<std::string> cmd_list();

        CommandServer() {
            this->com_register = {};
        }
};


#endif