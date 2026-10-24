#include "command.hpp"

#include <Arduino.h>

InputCommand::InputCommand(String input) {
    input.trim();
    input.toUpperCase();
    this->options = {};
    auto first_idx = input.indexOf(' ');
    if (first_idx == -1) {
        this->name = input;
    } else {
        this->name = input.substring(0, first_idx);
        input = input.substring(first_idx);
        input.trim();
        while (!input.isEmpty()) {
            first_idx = input.indexOf(' ');
            this->options.push_back(input.substring(0, first_idx));
            input = input.substring(first_idx);
            input.trim();
            yield();
        }
    }
}

CommandResult CommandBroker::interpretate(String cmd, String &output) {
    InputCommand input_command = InputCommand(cmd);
    auto cmdi = this->commands.find(input_command.name);
    if (cmdi == this->commands.end()) {
        if (input_command.name == "H") {
            String out = "Help:\n";
            for(auto i=this->commands.begin(); i!=this->commands.end(); i++) {
                out += i->second->help() + ";\n";
                yield();
            }
            output = out;
            return CommandResult::Done;
        } else {
            String out = "Unknown Command: " + input_command.name;
            output = out;
            return CommandResult::InvalidArgument;
        }
    } else {
        auto cur_command = cmdi->second;
        return cur_command->run(input_command, output);
    }
}