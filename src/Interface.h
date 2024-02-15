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
    // updates `timerISRHandler` periodically
    ESP8266Timer timerHandler {};
    ESP8266_ISR_Timer timerISRHandler {};

    // a boolean value used in determining whether the onboard LED should blink
    bool shouldLEDBlink {};

    bool sunriseTimerSet {};
    bool sunsetTimerSet {};

    Motor motor {};

    PushButton upBtn {};
    PushButton dnBtn {};

    void setLEDIndicator(bool state);
};

extern Interface interface;


#endif // AUTOBLINDSNODEMCU_INTERFACE_H
