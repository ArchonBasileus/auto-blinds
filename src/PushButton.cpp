#include "PushButton.h"

// return true on input which passes debounce check
bool PushButton::verifyExistingInput()
{
    // if button has input (as indicated by a non-zero `m_lastUpdate`) from at least 20ms after last input
    if (m_lastUpdate && (millis() - m_lastUpdate >= s_debounceDelayLen))
    {
        m_lastUpdate = 0;
        return digitalRead(m_pinNumber);
    }

    return false;
}