#ifndef CMD_HPP
#define CMD_HPP

#include <WString.h>
#include <functional>
#include <map>
#include <vector>

enum CommandResult {
    Done,
    AccessDenied,
    InvalidArgument,
    Internal
};

struct InputCommand {
    String name;
    std::vector<String> options;
    InputCommand(String input);
};

class Command;

class CommandBroker {
protected:
    std::map<String, Command*> commands;
public:
    virtual bool borrow(String mark) = 0;
    virtual bool is_borrowed() = 0;
    virtual void free() = 0;
    CommandResult interpretate(String cmd, String &output);
    CommandBroker(std::map<String, Command*> commands): commands(commands) {};
};

class Command{
protected:
    String _name;
    CommandBroker* broker;
public:
    Command(String name, CommandBroker* broker): _name(name), broker(broker) {};
    virtual CommandResult run(InputCommand input, String &output) = 0;
    virtual String help() = 0;
    String name() {return this->_name;}
    virtual bool stop() = 0;
};


#endif