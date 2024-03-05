#include "Motor.h"
#include <LittleFS.h>

// Called at program startup; sets the `AccelStepper` motor position to the one stored in flash memory.
// If no such `.bin` file exists, create one with the motor's upper bound as its position by default
void Motor::syncPositionFromFlash()
{
    // if motor position not stored in flash memory, create a file to store it in flash memory
    if (!LittleFS.exists(Constants::motorPositionFilePath))
    {
        setFlashPosition(static_cast<int>(Motor::s_stepRange));
        return;
    }

    m_system.setCurrentPosition(getFlashPosition());
}

void Motor::deriveInstruction(State instruction)
{
    // if motor is within boundaries and instruction was triggered by timer or by legal manual input
    if (m_state == stationary || m_state == -instruction)
        giveInstruction(instruction);
    else
        stopStepping();
}

bool Motor::withinBounds(State instruction)
{
    long currentPosition { m_system.currentPosition() };

    // `switch` statement necessary to confirm that flash-write is skipped when a move past boundaries is attempted
    switch (instruction)
    {
        case clockwise:
            return currentPosition < s_stepRange;
        case counterClockwise:
            return currentPosition > -s_stepRange;
        default:
            return false;
    }
}

// necessary to facilitate timed step instruction: without this, timer input would first have to pass input state rules
void Motor::giveInstruction(State instruction)
{
    if (withinBounds(instruction))
    {
        m_system.enableOutputs();
        m_state = instruction;
        m_system.moveTo(m_state * s_stepRange);
        m_system.setSpeed(s_topSpeed * static_cast<float>(m_state));
    }
}

void Motor::stopStepping()
{
    m_state = stationary;
    m_system.disableOutputs();

    // update motor's position in flash memory to reflect new position
    setFlashPosition(m_system.currentPosition());
}

void Motor::tryInstruction()
{
    // motor steps in instructed direction if state not stationary and step finishes inside set boundaries
    if (m_state != stationary)
    {
        if (m_system.distanceToGo())
            m_system.runSpeed();
        else
            stopStepping();
    }
}

void Motor::tryToggleSuspend()
{
    if (m_state != stationary)
        (m_suspended = !m_suspended) ? m_system.disableOutputs() : m_system.enableOutputs();
}