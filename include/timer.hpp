#ifndef TIMER_HPP
#define TIMER_HPP

#include <functional>
#include <Arduino.h>
#include "constants.hpp"

class TimerState {
  private:
  unsigned long last_tick;
  unsigned long timer;
  std::function<void(void)> run=NULL;
  bool on_run=false;

  public:
  unsigned long get_last_tick();

  void set_last_tick(unsigned long val);

  bool is_running();

  bool handle_timer(unsigned long time_stamp);

  TimerState(
    unsigned long last_tick,
    unsigned long timer,
    std::function<void(void)> run=[](){
      digitalWrite(MCU_LED, LOW);
      delay(2000); // 2 sec time work
      digitalWrite(MCU_LED, HIGH);
    }
  ):
    last_tick(last_tick), 
    timer(timer),
    run(run),
    on_run(false)
  { };
};

#endif