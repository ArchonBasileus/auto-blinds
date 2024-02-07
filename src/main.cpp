#include "APIHandling.h"
#include "ESP8266_ISR_Timer.h"

void IRAM_ATTR upBtnHandler();
void IRAM_ATTR dnBtnHandler();

[[maybe_unused]] void setup()
{
    // attach pin-change/push button interrupts
    attachInterrupt(digitalPinToInterrupt(interface.upBtn.getPinNumber()), upBtnHandler, RISING);
    attachInterrupt(digitalPinToInterrupt(interface.dnBtn.getPinNumber()), dnBtnHandler, RISING);

    // if Wi-Fi connection, filesystem initialisation and SSL certificates parsing successful
    if (initWiFi() && LittleFS.begin() && parseSSLCertificates())
    {
        // initialise NTP time client
        configTime(Constants::posixTimezone, Constants::ntpServerURL);

        // attach timer handler
        interface.interruptTimer.attachInterruptInterval(Constants::timerUpdateInterval, timerHandler);

        // set sunrise/sunset timers as required
        updateTimers();
    }
    // add more sophisticated error handling for all of the above section here
}

[[maybe_unused]] void loop()
{
    // poll 'up' button; if delay met, and interrupt triggered by contact
    if (interface.upBtn.verifyExistingInput())
        interface.motor.deriveInstruction(Motor::clockwise);

    // same as above, but for 'down' push button
    if (interface.dnBtn.verifyExistingInput())
        interface.motor.deriveInstruction(Motor::counterClockwise);

    // if no timers are set and Wi-Fi connection successful, update timers (add more guards)
    if (!(interface.sunriseTimerSet || interface.sunsetTimerSet) && initWiFi())
        updateTimers();

    // execute queued motor instruction
    interface.motor.runInstruction();
}

void upBtnHandler()
{
    interface.upBtn.resetDebounceTimer();
}

void dnBtnHandler()
{
    interface.dnBtn.resetDebounceTimer();
}