#include "FlashMemHandling.h"

CertStore g_certStore{};

bool parseSSLCertificates()
{
    int nCerts
    {
        g_certStore.initCertStore(LittleFS,PSTR("/certs.idx"),PSTR("/certs.ar"))
    };

    // add more sophisticated error handling
    if (!nCerts)
        return false;

    g_wiFiClient.setCertStore(&g_certStore);

    return true;
}

// returns motor's last flash-stored position as an integer
int getFlashPosition()
{
    File positionFile = LittleFS.open(Constants::motorPositionFilePath, "r");

    int readPosition {};
    positionFile.read((uint8_t*)&readPosition, sizeof(readPosition));

    positionFile.close();

    return readPosition;
}

void linkFlashPosition()
{
    // if motor position not stored in flash memory, create a file to store it in flash memory
    if (!LittleFS.exists(Constants::motorPositionFilePath))
    {
        setFlashPosition(static_cast<int>(Motor::s_stepRange));
        return;
    }

    interface.motor.system.setCurrentPosition(getFlashPosition());
}

void setFlashPosition(int newMotorPosition)
{
    File positionFile = LittleFS.open(Constants::motorPositionFilePath, "w");

    positionFile.write(reinterpret_cast<uint8_t*>(&newMotorPosition), sizeof(int));

    positionFile.close();
}
