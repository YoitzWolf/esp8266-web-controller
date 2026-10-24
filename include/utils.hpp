#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <WString.h>
#include <command.hpp>

std::string timestamp_to_str(unsigned long time_stamp);
std::string timestamp_to_str_time(unsigned long time_stamp);

float com_string_to_seconds(String s);

String comm_res_to_str(CommandResult r);

#endif