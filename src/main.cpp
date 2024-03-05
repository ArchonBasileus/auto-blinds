#include "FlashMemHandling.h"
#include "ESP8266_ISR_Timer.h"

// see initialisation of `g_interface` in `Interface.cpp` for other relevant program setup
[[maybe_unused]] void setup()
{
    // filesystem setup must be done within `setup()` for effects to persist throughout program
    if (!g_interface.trySetupFS())
        return;

    // initialise NTP time-client
    configTime(Constants::posixTimezone, Constants::ntpServerURL);

    // if no setup errors, timer switch enabled, and Wi-Fi connected, set sunrise/sunset timers
    if (g_interface.timerSwitch.isEnabled() && tryWiFiConnect())
        updateTimers();
}

void loop()
{
    // poll both push buttons; if legal input given since last check, send instruction to motor
    if (g_interface.upBtn.verifyExistingInput())
        g_interface.motor.deriveInstruction(Motor::clockwise);
    if (g_interface.dnBtn.verifyExistingInput())
        g_interface.motor.deriveInstruction(Motor::counterClockwise);

    // poll timer-toggling switch for a legal input
    if (g_interface.timerSwitch.verifyExistingInput())
        g_interface.toggleTimers();
    // if no timers are set (e.g., because they have expired), generate new ones
    else if (g_interface.timerSwitch.isEnabled() && !g_interface.areTimersSet() && tryWiFiConnect())
        updateTimers();

    // attempt to execute queued motor instruction
    g_interface.motor.tryInstruction();
}