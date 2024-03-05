#ifndef AUTOBLINDSESP8266_SWITCH_H
#define AUTOBLINDSESP8266_SWITCH_H


#include "PushButton.h"

class Switch : public PushButton
{
public:
    // use constructors from PushButton
    using PushButton::PushButton;

    bool verifyExistingInput() override;
    [[nodiscard]] bool isEnabled() const { return !m_state; }

private:
    bool m_state{ static_cast<bool>(digitalRead(m_pinNumber)) };

};


#endif // AUTOBLINDSESP8266_SWITCH_H