#ifndef AUTOBLINDSNODEMCU_MOTOR_H
#define AUTOBLINDSNODEMCU_MOTOR_H


#include <AccelStepper.h>

// forward declaration for function defined in `FlashMemHandling.cpp`
void setFlashPosition(int newMotorPosition);

class Motor
{
public:
    enum State
    {
        counterClockwise = -1,
        stationary,
        clockwise,
    };

    static constexpr auto s_stepRange { 19242 };
    static constexpr auto s_topSpeed { 500.0F };

    Motor() = default;

    Motor(uint8_t nDirPin, uint8_t nStepPin, uint8_t nEnablePin)
    {
        system = { AccelStepper::MotorInterfaceType::DRIVER,nStepPin, nDirPin };
        system.setEnablePin(nEnablePin);
        system.setPinsInverted(true, false, true);

        // set default step range to upper bound so that motor doesn't
        // attempt to pull past top of blinds in case of reset
        system.setCurrentPosition(s_stepRange);
        system.setMaxSpeed(s_topSpeed);
    }

    void deriveInstruction(State instruction);
    bool withinBounds();
    void tryStepping(State instruction);
    void stopStepping();
    void tryInstruction();

    AccelStepper system {};

private:
    State m_state { stationary };

};


#endif