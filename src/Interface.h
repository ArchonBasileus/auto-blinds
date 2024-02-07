#ifndef AUTOBLINDSNODEMCU_INTERFACE_H
#define AUTOBLINDSNODEMCU_INTERFACE_H


// selects a timer clock for use in timed interruptions
#define USING_TIM_DIV1                false           // for shortest and most accurate timer
#define USING_TIM_DIV16               false           // for medium time and medium accurate timer
#define USING_TIM_DIV256              true            // for longest timer but least accurate. Default

#include <ESP8266TimerInterrupt.h>
#include <ESP8266_ISR_Timer.hpp>
#include "Motor.h"
#include "PushButton.h"

struct Interface
{
    // updates `interruptCallbackHandler` periodically
    ESP8266Timer interruptTimer {};
    ESP8266_ISR_Timer interruptCallbackHandler {};

    bool sunriseTimerSet {};
    bool sunsetTimerSet {};

    Motor motor {};

    PushButton upBtn {};
    PushButton dnBtn {};
};

extern Interface interface;


#endif //AUTOBLINDSNODEMCU_INTERFACE_H
