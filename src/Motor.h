#ifndef AUTOBLINDSNODEMCU_MOTOR_H
#define AUTOBLINDSNODEMCU_MOTOR_H


#include <AccelStepper.h>
#include <LittleFS.h>

class Interface;

class Motor
{
public:
    enum State
    {
        counterClockwise = -1,
        stationary,
        clockwise,
    };

    static constexpr auto s_stepRange { 19542 };
    static constexpr auto s_topSpeed { 650.0F };

    Motor() = default;

    Motor(uint8_t nDirPin, uint8_t nStepPin, uint8_t nEnablePin)
    {
        m_system = {AccelStepper::MotorInterfaceType::DRIVER, nStepPin, nDirPin };
        m_system.setEnablePin(nEnablePin);
        m_system.setPinsInverted(false, false, true);
        m_system.setMaxSpeed(s_topSpeed);
        m_system.disableOutputs();
    }

    void syncPositionFromFlash();
    void deriveInstruction(State instruction);
    bool withinBounds(State instruction);
    void giveInstruction(State instruction);
    void stopStepping();
    void tryInstruction();

    // have another look at these
    void tryToggleSuspend();

private:
    AccelStepper m_system {};
    State m_state { stationary };
    bool m_suspended{};

};

namespace Constants
{
    inline constexpr auto motorPositionFilePath { "/motorPosition.bin" };
}

int getFlashPosition();
void setFlashPosition(int newMotorPosition);

#endif