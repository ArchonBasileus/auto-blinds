#include "APIHandling.h"

constexpr auto apiURLPrefixLen { 83 };
constexpr auto apiURLSuffixLen { 11 };
WiFiClientSecure g_wifiClient{};
CertStore g_certStore{};

// a static-length character array which holds the API's URL
char g_apiURL[apiURLPrefixLen + apiURLSuffixLen]
{
    "https://api.sunrisesunset.io/json?lat=XXXXXXXXXX&lng=XXXXXXXXXX&timezone=XXXX&date=YYYY-MM-DD"
};

void timerHandler()
{
    interface.interruptCallbackHandler.run();
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

void updateLEDIndicator()
{
    static auto s_delayTimerStart { 0UL };

    if (millis() - s_delayTimerStart >= Constants::ledBlinkDelay)
    {
        static auto ledState { false };

        digitalWrite(LED_BUILTIN, (ledState = !ledState));
        s_delayTimerStart = millis();
    }
}

bool initWiFi()
{
    static auto s_lastWiFiConnectAttempt { 0UL };

    if (WiFi.status() == WL_CONNECTED)
        return true;
    // if Wi-Fi not connected, cease attempts to connect if it has not been at least an hour since last attempt
    else if (millis() - s_lastWiFiConnectAttempt > Constants::attemptWiFiCooldownLen)
        return false;


    WiFi.begin(Constants::ssid, Constants::pw);
    unsigned long wiFiConnectStartTime { millis() };

    while (WiFi.status() != WL_CONNECTED
           && !WiFi.localIP()
           && (millis() - wiFiConnectStartTime <= Constants::wiFiTimeoutLen))
    {
        updateLEDIndicator();
        yield();
    }

    // potentially add handling to make error diagnostics easier

    return (WiFi.status() == WL_CONNECTED);
}

bool parseSSLCertificates()
{
    int nCerts
    {
        g_certStore.initCertStore(LittleFS,PSTR("/certs.idx"),PSTR("/certs.ar"))
    };

    // add more sophisticated error handling
    if (!nCerts)
        return false;

    g_wifiClient.setCertStore(&g_certStore);

    return true;
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
    // add error handler
}

JsonDocument getApiJSON(const std::tm& currentTime)
{
    updateApiURL(currentTime);

    HTTPClient httpClient{};
    httpClient.begin(g_wifiClient, g_apiURL);

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
    // Serial.printf("HTTPResponse/SSLError codes: %d/%d\n", respCode, g_wifiClient.getLastSSLError());

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

    apiJSON.clear();
    
    return timerInfo;
}

bool updateTimers()
{
    // enable onboard LED to indicate that NTP server and API may be polled, which could take a few seconds
    digitalWrite(LED_BUILTIN, LOW);

    // add more sophisticated error handling
    TimerUpdateData timerInfo { getTimerInfo() };

    // blocking internet functions finished: disable connecting/parsing LED indicator
    digitalWrite(LED_BUILTIN, HIGH);

    // thinks code never reached bc it never sees that `timerRequired` is in scope with a potentially non-zero value
    // debug timerInfo
    // TimerUpdateData timerInfo { .timersRequired = 1, .sunriseTimerLen = -22, .sunsetTimerLen = 20'000 };

    if (timerInfo.timersRequired > 0)
    {
        if (interface.interruptCallbackHandler.setTimeout(timerInfo.sunsetTimerLen, sunsetHandler) != -1)
            interface.sunsetTimerSet = true;
        else
            return false;

        if (timerInfo.timersRequired > 1)
        {
            if (interface.interruptCallbackHandler.setTimeout(timerInfo.sunriseTimerLen, sunriseHandler) != -1)
                interface.sunriseTimerSet = true;
            else
                return false;
        }
    }

    return true;
}