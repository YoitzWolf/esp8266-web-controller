
#include "timer.hpp"

unsigned long TimerState::get_last_tick() {
    return this->last_tick;
}

void TimerState::set_last_tick(unsigned long val) {
    this->last_tick = val;
}

bool TimerState::is_running() {
    return this->on_run;
}

void TimerState::finish() {
    this->on_run = false;
    // if (this->ntc_client != NULL) {
    //     this->last_tick = this->ntc_client->getEpochTime();
    // }
}

void TimerState::start() {
    this->on_run = true;
    if (this->ntc_client != NULL) {
        this->last_tick = this->ntc_client->getEpochTime();
    }
}

void TimerState::callback() {
    if (this->long_tick) {
        ++this->long_tick->count;
        if (this->long_tick->count < this->long_tick->total) {
            return;
        }
    }
    this->start();
    this->run();
    this->finish();
    if (this->repeat) {
        if (this->long_tick) {
            this->long_tick->count = 0;
        }
        return;
    } else {
        os_timer_disarm(&this->_timer_internal);
        this->long_tick.reset(nullptr);
    }
}

bool TimerState::handle_timer() {
    os_timer_disarm(&_timer_internal);
    ets_timer_setfn(
        &_timer_internal,
        [](void* ptr) {
            auto tmr = reinterpret_cast<TimerState*>(ptr);
            tmr->callback();
        }, this
    );
    size_t total = 0;
    size_t milliseconds = this->time_in_ms;
    if (milliseconds > DURATION_MAX) {
        total = 1;
        while (milliseconds > DURATION_MAX) {
            total *= 2;
            milliseconds /= 2;
        }
        this->long_tick.reset(new TimerState::LongTicker{
            .total = total,
            .count = 0,
        });
        repeat = true;
    }
    os_timer_arm(&_timer_internal, milliseconds, this->repeat);

    return false;
}