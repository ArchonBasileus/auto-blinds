#ifndef AUTOBLINDSNODEMCU_INTERFACE_H
#define AUTOBLINDSNODEMCU_INTERFACE_H


// selects a timer clock for use in timed interruptions
#define USING_TIM_DIV1                false           // for shortest and most accurate timer
#define USING_TIM_DIV16               false           // for medium time and medium accurate timer
#define USING_TIM_DIV256              true            // for longest timer but least accurate. Default

#include <ESP8266TimerInterrupt.h>
#include <ESP8266_ISR_Timer.hpp>
#include <utility>
#include "Motor.h"
#include "Switch.h"

class Interface
{
public:
    enum TimerType
    {
        sunrise,
        sunset,
    };

    static constexpr auto ledBlinkInterval{ 250UL };
    static constexpr auto timerUpdateInterval{ 50'000L };

    Interface(const Motor&& motorObject,
              const PushButton&& upBtnObject,
              const PushButton&& dnBtnObject,
              const Switch&& timerSwitchObject);

    // acts upon parameter `state` to either begin or cease blinking the onboard LED
    // to indicate that an input or motor-blocking operation is being performed
    void toggleLoadingIndicator();
    bool trySetTimer(unsigned long timerLen, TimerType timerType = sunrise);
    void clearTimers();
    void toggleTimers();
    [[nodiscard]] bool areTimersSet() { return m_timerISRHandler.getNumTimers() > 0; }
    [[nodiscard]] bool isIndicatorOn() const { return m_loadingIndicatorOn; }

    Motor motor{};

    PushButton upBtn{};
    PushButton dnBtn{};

    Switch timerSwitch{};

private:
    friend void setup();
    friend void timerCallback();
    friend void sunriseHandler();
    friend void sunsetHandler();

    // necessary because `parseSSLCertificates` only works if called in `setup()`
    bool trySetupFS();

    // updates `m_timerISRHandler` periodically
    ESP8266Timer m_timerHandler{};
    ESP8266_ISR_Timer m_timerISRHandler{};

    // a boolean value used in determining whether the onboard LED should blink
    bool m_loadingIndicatorOn{ true };

};

extern Interface g_interface;


#endif // AUTOBLINDSNODEMCU_INTERFACE_H
