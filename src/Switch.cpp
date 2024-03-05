#include "Switch.h"

bool Switch::verifyExistingInput()
{
    if (m_lastUpdate && (millis() - m_lastUpdate >= s_debounceDelayLen))
    {
        m_lastUpdate = 0;

        // input is valid if pin state reads the opposite of recorded switch state after settling
        if (digitalRead(m_pinNumber) != m_state)
        {
            m_state = !m_state;
            return true;
        }
    }

    return false;
}