#include "FlashMemHandling.h"
#include "ESP8266_ISR_Timer.h"

void IRAM_ATTR upBtnHandler();
void IRAM_ATTR dnBtnHandler();

[[maybe_unused]] void setup()
{
    // configure onboard LED as an output pin
    pinMode(LED_BUILTIN, OUTPUT);

    // attach timer callback handler
    interface.timerHandler.attachInterruptInterval(Constants::timerUpdateInterval, timerCallback);

    // attach pin-change/push button interrupts
    attachInterrupt(digitalPinToInterrupt(interface.upBtn.getPinNumber()), upBtnHandler, RISING);
    attachInterrupt(digitalPinToInterrupt(interface.dnBtn.getPinNumber()), dnBtnHandler, RISING);

    // ensure filesystem mount is successful before continuing
    if (LittleFS.begin())
    {
        // start blinking the input/motor-blocking operation LED indicator
        interface.setLEDIndicator(true);

        linkFlashPosition();

        // if SSL certificate parsing and Wi-Fi initialisation successful, set sunrise/sunset timers
        if (parseSSLCertificates() && initWiFi())
        {
            // initialise NTP time client
            configTime(Constants::posixTimezone, Constants::ntpServerURL);

            updateTimers();
        }
    }
    // add more sophisticated error handling
}

[[maybe_unused]] void loop()
{
    // poll the 'up'/'down' push-buttons, and verify any existing inputs before accepting them
    if (interface.upBtn.verifyExistingInput())
        interface.motor.deriveInstruction(Motor::clockwise);
    if (interface.dnBtn.verifyExistingInput())
        interface.motor.deriveInstruction(Motor::counterClockwise);

    // if no timers are set and Wi-Fi connection successful, update timers (add more guards)
    if (!(interface.sunriseTimerSet || interface.sunsetTimerSet) && initWiFi())
        updateTimers();

    // attempt to execute queued motor instruction
    interface.motor.tryInstruction();
}

void upBtnHandler()
{
    interface.upBtn.resetDebounceTimer();
}

void dnBtnHandler()
{
    interface.dnBtn.resetDebounceTimer();
}