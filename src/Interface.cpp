#include "Interface.h"

void IRAM_ATTR timerCallback();
void IRAM_ATTR sunriseHandler();
void IRAM_ATTR sunsetHandler();
void IRAM_ATTR upBtnHandler();
void IRAM_ATTR dnBtnHandler();
void IRAM_ATTR timerSwitchHandler();
bool parseSSLCertificates();
bool tryWiFiConnect();
bool updateTimers();

Interface::Interface(const Motor&& motorObject,
                     const PushButton&& upBtnObject,
                     const PushButton&& dnBtnObject,
                     const Switch&& timerSwitchObject)
    : motor{ motorObject }
    , upBtn{ upBtnObject }
    , dnBtn{ dnBtnObject }
    , timerSwitch{ timerSwitchObject }
{
    // configures LED as an output pin and turns it on
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    // attaches timer update interruption
    m_timerHandler.attachInterruptInterval(timerUpdateInterval, timerCallback);

    // attach pin-change interrupts for push-buttons and timer switch
    attachInterrupt(digitalPinToInterrupt(upBtn.m_pinNumber), upBtnHandler, RISING);
    attachInterrupt(digitalPinToInterrupt(dnBtn.m_pinNumber), dnBtnHandler, RISING);
    attachInterrupt(digitalPinToInterrupt(timerSwitch.m_pinNumber), timerSwitchHandler, CHANGE);
}

// Blink onboard LED rapidly and halt program if SSL certificate-parsing or motor position syncing fails.
bool Interface::trySetupFS()
{
    // assert flash-memory-related setup success before continuing setup
    if (LittleFS.begin() && parseSSLCertificates())
    {
        motor.syncPositionFromFlash();
        return true;
    }
    // blink very quickly to indicate error, and cease program execution
    else
        return false;
}

void Interface::toggleLoadingIndicator()
{
    motor.tryToggleSuspend();
    // invert `val` argument to `digitalWrite()` because `LED_BUILTIN` signals are inverted
    digitalWrite(LED_BUILTIN, !(m_loadingIndicatorOn = !m_loadingIndicatorOn));
}

bool Interface::trySetTimer(unsigned long timerLen, TimerType timerType)
{
    void(*timerCallback)(){ sunriseHandler };

    if (timerType == sunset)
        timerCallback = sunsetHandler;

    return (m_timerISRHandler.setTimeout(timerLen, timerCallback) != -1);
}

void Interface::clearTimers()
{
    // if timed interruptions have just been disabled, clear existing timers
    for (int i{ 0 }; i < m_timerISRHandler.getNumTimers(); i++)
        m_timerISRHandler.deleteTimer(i);
}

void Interface::toggleTimers()
{
    // if toggled on, attempt to update timers if none are set and Wi-Fi connects
    if (timerSwitch.isEnabled())
    {
        if (tryWiFiConnect())
            updateTimers();
    }
    // if toggled off: clear existing timers
    else
        clearTimers();
}

void timerCallback()
{
    g_interface.m_timerISRHandler.run();

    if (!g_interface.m_loadingIndicatorOn)
        return;

    static auto s_delayTimerStart { 0UL };

    if (millis() - s_delayTimerStart >= Interface::ledBlinkInterval)
    {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        s_delayTimerStart = millis();
    }
}

void sunriseHandler()
{
    g_interface.motor.giveInstruction(Motor::clockwise);
}

void sunsetHandler()
{
    g_interface.motor.giveInstruction(Motor::counterClockwise);
}

void upBtnHandler()
{
    g_interface.upBtn.resetDebounceTimer();
}

void dnBtnHandler()
{
    g_interface.dnBtn.resetDebounceTimer();
}

void timerSwitchHandler()
{
    g_interface.timerSwitch.resetDebounceTimer();
}

// sets up the program's interface: configures onboard LED, and attaches timer/input interrupts
Interface g_interface
{
    { D8, D6, D7 },
    { PushButton{ D1 } },
    { PushButton{ D2 } },
    { Switch{ D5 } }
};