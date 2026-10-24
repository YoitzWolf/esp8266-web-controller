#ifndef RELAYBROKER_HPP
#define RELAYBROKER_HPP


#include <Arduino.h>
#include <Ticker.h>
// #include "timer.hpp"
#include "command.hpp"


class DropBorrow: public Command {
public:
    DropBorrow(CommandBroker* broker):
        Command("D", broker)
    { };
    CommandResult run(InputCommand input, String &output) override {
        this->broker->free();
        return Done;
    };
    String help() override { return "Stop all command run and turn off."; };
    bool stop() override {return true;}
};

class SetSignalCommand: public Command {
protected:
    std::function<void(uint8_t)> set_signal;
    uint8_t on;
    uint8_t off;
public:
    SetSignalCommand(CommandBroker* broker, uint8_t on, uint8_t off, std::function<void(uint8_t)> set_signal):
        Command("S", broker),
        set_signal(set_signal),
        on(on),
        off(off)
    { };
    CommandResult run(InputCommand input, String &output) override;
    String help() override;
    bool stop() override {return true;}
};

class RunTimerCommand: public Command {
protected:
    std::function<void(uint8_t)> set_signal;
    uint8_t on;
    uint8_t off;
    Ticker ticker;
    Ticker ticker_2;
public:
    RunTimerCommand(CommandBroker* broker, uint8_t on, uint8_t off, std::function<void(uint8_t)> set_signal):
        Command("RT", broker),
        set_signal(set_signal),
        on(on),
        off(off),
        ticker(Ticker()),
        ticker_2(Ticker())
    { };
    CommandResult run(InputCommand input, String &output) override;
    String help() override;
    bool stop() override;
};


class RelayBroker: public CommandBroker {
protected:
    std::map<String, Command*>::iterator borrowing;
    uint8_t pin;
    uint8_t ON;
    uint8_t OFF;
    void send_on() { digitalWrite(pin, ON); } ;
    void send_off() { digitalWrite(pin, OFF); };
public:
    RelayBroker(uint8_t pin, uint8_t ON, uint8_t OFF, std::map<String, Command*> commands): CommandBroker(commands), pin(pin), ON(ON), OFF(OFF) {
        Command* signal = new SetSignalCommand(
            this, ON, OFF, [pin](uint8_t sig) { digitalWrite(pin, sig); }
        );
        this->commands.insert(
            {signal->name(), (Command*) signal}
        );
        Command* timer = new RunTimerCommand(
            this, ON, OFF, [pin](uint8_t sig) { digitalWrite(pin, sig); }
        );
        this->commands.insert(
            {timer->name(), (Command*) timer}
        );
        Command* drop = new DropBorrow(
            this
        );
        this->commands.insert(
            {drop->name(), (Command*) drop}
        );
        this->borrowing = this->commands.end();
    };
    bool borrow(String mark) override {
        if (this->is_borrowed()) {
            return false;
        }
        this->borrowing=this->commands.find(mark);
        return this->borrowing!=commands.find(mark);
    }
    void free() override {
        this->send_off();
        if(this->borrowing != this->commands.end()) {
            this->borrowing->second->stop();
            this->borrowing=this->commands.end();
        }
    }
    bool is_borrowed() override {
        return (this->borrowing != this->commands.end());
    }
};

#endif