#include "InternetHandling.h"

constexpr auto apiURLPrefixLen { 83 };
constexpr auto apiURLSuffixLen { 11 };

WiFiClientSecure g_wiFiClient {};

// a static-length character array which holds the API's URL
char g_apiURL[apiURLPrefixLen + apiURLSuffixLen]
{
    "https://api.sunrisesunset.io/json?lat=XXXXXXXXXX&lng=XXXXXXXXXX&timezone=XXXX&date=YYYY-MM-DD"
};

void timerCallback()
{
    interface.timerISRHandler.run();

    if (!interface.shouldLEDBlink)
        return;

    static auto s_delayTimerStart { 0UL };

    if (millis() - s_delayTimerStart >= Constants::ledBlinkDelay)
    {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        s_delayTimerStart = millis();
    }
}

void sunriseHandler()
{
    interface.motor.tryStepping(Motor::clockwise);
    interface.sunriseTimerSet = false;
}

void sunsetHandler()
{
    interface.motor.tryStepping(Motor::counterClockwise);
    interface.sunsetTimerSet = false;
}

bool initWiFi()
{
    static auto s_lastWiFiConnectAttempt { 0UL };

    if (WiFi.status() == WL_CONNECTED)
        return true;
    // if Wi-Fi not connected, cease attempts to connect if it has not been at least an hour since last attempt
    else if (millis() - s_lastWiFiConnectAttempt > Constants::attemptWiFiCooldownLen)
        return false;

    // indicate beginning of program-blocking internet operations via blinking LED
    interface.setLEDIndicator(true);

    WiFi.begin(Constants::ssid, Constants::pw);
    unsigned long wiFiConnectStartTime { millis() };

    while (WiFi.status() != WL_CONNECTED
           && !WiFi.localIP()
           && (millis() - wiFiConnectStartTime <= Constants::wiFiTimeoutLen))
    {
        timerCallback();
        yield();
    }

    // potentially add handling to make error diagnostics easier

    bool isWiFiConnected { WiFi.status() == WL_CONNECTED };

    // disable blinking LED indicator if no connection
    if (!isWiFiConnected)
        interface.setLEDIndicator(false);

    return (isWiFiConnected);
}

int parseTimeField(const char*& timeString, char delimiter)
{
    // the `10` argument to `strtol` describes the numeral system used in `timeString` (decimal/base 10 here)
    int timeFieldValue { strtol(timeString, const_cast<char**>(&timeString), 10) };

    if (*timeString == delimiter)
    {
        timeString++;

        return timeFieldValue;
    }

    // add more sophisticated error handling
    return -1;
}

int parseTimeToMilliseconds(int hours, int minutes, int seconds)
{
    return ((hours * 3'600'000) + (minutes * 60'000) + (seconds * 1000));
}

int parseTimeToMilliseconds(const char* timeString)
{
    long hours { parseTimeField(timeString, ':') };
    long minutes { parseTimeField(timeString, ':') };
    long seconds { parseTimeField(timeString, ' ') };

    char periodIndicator { *timeString };

    hours = (periodIndicator == 'P' && hours != 12) ? hours + 12 :
            (periodIndicator == 'A' && hours == 12) ? 0 : hours;

    return parseTimeToMilliseconds(hours, minutes, seconds);
}

std::tm getCurrentTime()
{
    std::tm currentTime {};
    getLocalTime(&currentTime);

    return currentTime;
}

void updateApiURL(const std::tm& time)
{
    char formattedDate[apiURLSuffixLen];

    if (strftime(formattedDate, apiURLSuffixLen, "%Y-%m-%d", &time) > 0)
    {
        std::strncpy(&g_apiURL[apiURLPrefixLen], formattedDate, apiURLSuffixLen);
        g_apiURL[apiURLPrefixLen + apiURLSuffixLen - 1] = '\0';
    }
    // add more sophisticated error handling
}

JsonDocument getApiJSON(const std::tm& currentTime)
{
    updateApiURL(currentTime);

    HTTPClient httpClient{};
    httpClient.begin(g_wiFiClient, g_apiURL);

    int respCode{ httpClient.GET() };

    JsonDocument apiJSON {};
    DeserializationError dsError {};

    // successful API poll, add more sophisticated error handling
    if (respCode > 0 && respCode < 400)
        dsError = deserializeJson(apiJSON, httpClient.getStream());

    httpClient.end();

    // add more sophisticated dsError handling
    if (dsError)
    {
        // Serial.print(F("deserializeJson() failed: "));
        // Serial.println(dsError.c_str());
        return JsonDocument{};
    }

    // TO BE REMOVED:
    // Serial.printf("HTTPResponse/SSLError codes: %d/%d\n", respCode, g_wiFiClient.getLastSSLError());

    return apiJSON;
}

TimerUpdateData getTimerInfo()
{
    TimerUpdateData timerInfo {};

    if (interface.sunsetTimerSet && !interface.sunriseTimerSet)
        return timerInfo;

    tm ntpTime { getCurrentTime() };
    // add more sophisticated error handling
    int milliSecsElapsedToday = parseTimeToMilliseconds(ntpTime.tm_hour,
                                                        ntpTime.tm_min,
                                                        ntpTime.tm_sec);

    JsonDocument apiJSON {};

    if (!(apiJSON = getApiJSON(ntpTime)).isNull())
    {
        timerInfo.sunriseTimerLen = parseTimeToMilliseconds(apiJSON["results"]["sunrise"].as<const char *>())
                                    - milliSecsElapsedToday;
        timerInfo.sunsetTimerLen = parseTimeToMilliseconds(apiJSON["results"]["sunset"].as<const char *>())
                                   - milliSecsElapsedToday;
    }
    // add more sophisticated error handling
    else
    {
        // Serial.println("HTTP GET returned a null JSON");
        return TimerUpdateData {};
    }

    if (timerInfo.sunsetTimerLen <= 0)
    {
        ntpTime.tm_mday++;
        mktime(&ntpTime);

        apiJSON.clear();
        apiJSON = getApiJSON(ntpTime);

        if (apiJSON.isNull())
            return TimerUpdateData {};

        int remainingMilliSecsToday {Constants::milliSecsPerDay - milliSecsElapsedToday };

        timerInfo.sunriseTimerLen = remainingMilliSecsToday
                                    + parseTimeToMilliseconds(
                                            apiJSON["results"]["sunrise"].as<const char*>());
        timerInfo.sunsetTimerLen = remainingMilliSecsToday
                                   + parseTimeToMilliseconds(apiJSON["results"]["sunset"].as<const char*>());

        // for use in calculating number of required timers, current
        // time today is represented as negative time tomorrow
        milliSecsElapsedToday = -remainingMilliSecsToday;
    }

    timerInfo.timersRequired = ((timerInfo.sunriseTimerLen - milliSecsElapsedToday) <= 0) ? 1 : 2;

    return timerInfo;
}

bool updateTimers()
{
    // add more sophisticated error handling
    TimerUpdateData timerInfo { getTimerInfo() };

    // input/motor-blocking operations finished: disable the onboard LED blink indicator
    interface.setLEDIndicator(false);

    if (timerInfo.timersRequired > 0)
    {
        if (interface.timerISRHandler.setTimeout(timerInfo.sunsetTimerLen, sunsetHandler) != -1)
        {
            interface.sunsetTimerSet = true;
            // Serial.printf("sunset timer length = %d seconds\n", timerInfo.sunsetTimerLen / 1000);
        }
        else
            return false;

        if (timerInfo.timersRequired > 1)
        {
            if (interface.timerISRHandler.setTimeout(timerInfo.sunriseTimerLen, sunriseHandler) != -1)
            {
                interface.sunriseTimerSet = true;
                // Serial.printf("sunrise timer length = %d seconds\n", timerInfo.sunriseTimerLen / 1000);
            }
            else
                return false;
        }
    }

    return true;
}