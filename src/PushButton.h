#ifndef AUTOBLINDSNODEMCU_PUSHBUTTON_H
#define AUTOBLINDSNODEMCU_PUSHBUTTON_H


#include <Arduino.h>
class Interface;
class Motor;
class Switch;

class PushButton
{
public:
    static constexpr auto s_debounceDelayLen {20UL };

    PushButton() = default;

    explicit PushButton(uint8_t pin)
            : m_pinNumber { pin }
    {}

    virtual bool verifyExistingInput();
    void resetDebounceTimer() { m_lastUpdate = millis(); }

protected:
    friend class Interface;
    /*
    friend Interface::Interface(const Motor &&motorObject,
                                const PushButton &&upBtnObject,
                                const PushButton &&dnBtnObject,
                                const Switch &&timerSwitchObject);
    */

    uint8_t m_pinNumber {};
    volatile unsigned long m_lastUpdate {};

};


#endif