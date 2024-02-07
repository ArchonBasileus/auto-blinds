#include "Motor.h"

void Motor::deriveInstruction(State instruction)
{
    // if motor is within boundaries and instruction was triggered by timer or by legal manual input
    if (withinBounds() && (m_state == stationary || m_state == -instruction))
        tryStepping(instruction);
    else
        stopStepping();
}

bool Motor::withinBounds()
{
    long currentPosition { system.currentPosition() };

    return ((currentPosition <= s_stepRange) && (currentPosition >= -s_stepRange));
}

void Motor::tryStepping(State instruction)
{
    if (withinBounds())
    {
        system.enableOutputs();
        m_state = instruction;
        system.moveTo(m_state * s_stepRange);
        system.setSpeed(s_topSpeed * static_cast<float>(m_state));
    }
}

void Motor::stopStepping()
{
    m_state = stationary;
    system.disableOutputs();
}

void Motor::runInstruction()
{
    // motor steps in instructed direction if state not stationary and step finishes inside set boundaries
    if (m_state != stationary && system.distanceToGo())
        system.runSpeed();
    else
        stopStepping();
}