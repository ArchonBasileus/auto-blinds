#ifndef AUTOBLINDSNODEMCU_PUSHBUTTON_H
#define AUTOBLINDSNODEMCU_PUSHBUTTON_H


#include <Arduino.h>

class PushButton
{
public:
    static constexpr auto s_debounceDelay { 20UL };

    PushButton() = default;

    explicit PushButton(uint8_t pin)
            : m_pinNumber { pin }
    {}

    [[nodiscard]] uint8_t getPinNumber() const { return m_pinNumber; }
    void resetDebounceTimer() { m_lastUpdate = millis(); }
    bool verifyExistingInput();

private:
    uint8_t m_pinNumber {};
    unsigned long m_lastUpdate {};

};


#endif
