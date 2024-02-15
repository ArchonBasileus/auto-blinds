#ifndef AUTOBLINDSNODEMCU_INTERNETHANDLING_H
#define AUTOBLINDSNODEMCU_INTERNETHANDLING_H


#include <ctime>
#include <sstream>
#include <iomanip>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Interface.h"

struct TimerUpdateData
{
    int timersRequired {};
    int sunriseTimerLen {};
    int sunsetTimerLen {};
};

namespace Constants
{
    inline constexpr auto ssid
    {
        "ssid"
    };
    inline constexpr auto pw
    {
        "pw"
    };
    inline constexpr auto ntpServerURL { "pool.ntp.org" };
    inline constexpr auto posixTimezone { "XXXX-XXXXXX,XXX.X.X,XX.X.X" };
    // the below integral values should be interpreted in millisecond units
    inline constexpr auto wiFiTimeoutLen { 15'000UL };
    inline constexpr auto timerUpdateInterval { 50'000L };
    inline constexpr auto milliSecsPerDay { 86'400'000 };
    inline constexpr auto ledBlinkDelay { 250UL };
    inline constexpr auto attemptWiFiCooldownLen { 3'600'000UL };
}

extern WiFiClientSecure g_wiFiClient;

void IRAM_ATTR timerCallback();
void IRAM_ATTR sunriseHandler();
void IRAM_ATTR sunsetHandler();
bool initWiFi();
bool updateTimers();

#endif