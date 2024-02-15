#include "Interface.h"

// acts upon parameter `state` to either begin or cease blinking the onboard LED
// to indicate that an input or motor-blocking operation is being performed
void Interface::setLEDIndicator(bool state)
{
    digitalWrite(LED_BUILTIN, !state);
    shouldLEDBlink = state;
}

Interface interface { .motor { D5, D6, D7 }, .upBtn { PushButton { D1 } }, .dnBtn { PushButton { D2 } } };