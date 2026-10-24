
#include "utils.hpp"
#include <WString.h>
#include <command.hpp>

unsigned long __daysOfMonth[] = { 31, 28, 31, 30, 31, 30,
                          31, 31, 30, 31, 30, 31 };

std::string timestamp_to_str(unsigned long time_stamp) {
    unsigned long hours   = ((time_stamp % 86400L) / 3600);
    unsigned long minutes = ((time_stamp % 3600L) / 60);
    unsigned long seconds = ( time_stamp % 60);
    // -
    unsigned long days_total = time_stamp / 86400L;
    unsigned long four_years = days_total / 1461;
    unsigned long local_year_days = days_total % 1461;
    unsigned long day = local_year_days % 365;
    unsigned long year = 1970 + four_years*4 + (local_year_days)/365;
    unsigned long month = 0;
    unsigned long s = 0;
    unsigned long mon_day = day + 1;
    while (s + __daysOfMonth[month] + (year % 4 == 0 && month==2 ? 1:0) <= day) {
        s += __daysOfMonth[month] + (year % 4 == 0 && month==2 ? 1:0);
        mon_day -= __daysOfMonth[month] + (year % 4 == 0 && month==2 ? 1:0);
        month++;
    }
    month++;
    char bufstring[32];
    std::sprintf(bufstring, "%02d.%02d.%d / %02d:%02d:%02d", mon_day, month, year, hours, minutes, seconds);
    std::string sf = bufstring;
    return sf;
}

std::string timestamp_to_str_time(unsigned long time_stamp) {
    unsigned long hours   = ((time_stamp) / 3600);
    unsigned long minutes = ((time_stamp % 3600L) / 60);
    unsigned long seconds = ( time_stamp % 60);
    char bufstring[32];
    std::sprintf(bufstring, "%d:%02d:%02d", hours, minutes, seconds);
    std::string sf = bufstring;
    return sf;
}

float com_string_to_seconds(String s) {
    if (s.length() < 2) {
        return -1;
    }
    auto digit = s.substring(0, s.length()-1).toFloat();
    auto mark = s.end();
    if (*mark == 'm') {
        digit *= 60;
    } else if (*mark == 'h') {
        digit *= 60*60;
    } else if (*mark == 'm') {
        digit *= 60*60*24;
    }
    return (digit);
}

String comm_res_to_str(CommandResult r) {
    switch (r)
    {
        case Done:
            return "Done";
            break;
        case AccessDenied:
            return "AccessDenied";
            break;
        case InvalidArgument:
            return "InvalidArgument";
            break;
        case Internal:
            return "Internal";
            break;
    }
    return "Unknown Error";
}