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