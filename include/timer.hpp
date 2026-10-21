#ifndef TIMER_HPP
#define TIMER_HPP

#include <functional>
#include <memory>
#include <Arduino.h>
#include <NTPClient.h>

#include "constants.hpp"

static constexpr auto DURATION_MAX = 6870947;

class TimerState
{
private:
    NTPClient *ntc_client; // = nullptr;
    unsigned long last_tick;
    unsigned long time_in_ms;
    std::function<void(void)> run; // = NULL;
    bool on_run = false;
    ETSTimer _timer_internal{};
    bool repeat = false;

protected:
    struct LongTicker
    {
        uint32_t total = 0;
        uint32_t count = 0;
    };
    std::unique_ptr<LongTicker> long_tick;

    void finish();
    void start();
    void callback();

public:
    unsigned long get_last_tick();

    unsigned long period_ms(){
        return this->time_in_ms;
    };

    /**
     * Return last epoch, when timer called its callback
     */
    void set_last_tick(unsigned long val);

    bool is_running();

    bool is_repeat() { return this->repeat; };

    /**
     * Run timer
     */
    bool handle_timer();

    bool fast_call();

    TimerState(
        unsigned long time_in_ms,
        std::function<void(void)> run,
        NTPClient *ntc_client,
        bool repeat = false,
        unsigned long last_tick=0
    ) : last_tick(last_tick),
        time_in_ms(time_in_ms),
        run(run),
        ntc_client(ntc_client),
        on_run(false),
        repeat(repeat)
    {
        this->long_tick.reset(nullptr);
    };
};

#endif