#ifndef AUTOBLINDSESP8266_FLASHMEMHANDLING_H
#define AUTOBLINDSESP8266_FLASHMEMHANDLING_H


#include <LittleFS.h>
#include "InternetHandling.h"

namespace Constants
{
    inline constexpr auto motorPositionFilePath { "/motorPosition.bin" };
}

bool parseSSLCertificates();
void linkFlashPosition();

#endif // AUTOBLINDSESP8266_FLASHMEMHANDLING_H
