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
    int timersRequired{};
    int sunriseTimerLen{};
    int sunsetTimerLen{};
};

namespace Constants
{
    inline constexpr auto ssid
    {
        "..."
    };
    inline constexpr auto pw
    {
        "..."
    };
    inline constexpr auto ntpServerURL { "pool.ntp.org" };
    inline constexpr auto posixTimezone { "AEST-10AEDT,M10.1.0,M4.1.0" };
    // the below integral values should be interpreted in millisecond units
    inline constexpr auto wiFiTimeoutLen { 15'000UL };
    inline constexpr auto milliSecsPerDay { 86'400'000 };
    inline constexpr auto tryWiFiCooldownLen { 3'600'000UL };
}

extern WiFiClientSecure g_wiFiClient;

bool tryWiFiConnect();
bool updateTimers();

#endif
