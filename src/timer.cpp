
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

bool TimerState::handle_timer(unsigned long time_stamp) {
    auto delta = time_stamp - this->last_tick;
    if (delta >= this->timer) { // every 10 secs
      Serial.println("Run timer event");
      this->set_last_tick(time_stamp);
      this->on_run = true;
      this->run();
      this->on_run = false;
      return true;
    }
    return false;
}