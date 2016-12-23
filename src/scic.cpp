#include "scic.h"
#include "global.h"
#include "keypad.h"
//#include "cpu.h"

Scic *scic;

void Scic::init()
{
    nextNMI = 921600;
    stage2 = 0;
    NMItoCPU = false;
    acout = false;
    pulse = false;
}

// Switch alarm on and off. Would control buzzer, but not
// currently emulated.
void Scic::setAlarm(bool value)
{
    alarm = value;
}

void Scic::setPulse(bool value)
{
    pulse = value;
}

// Set NMI events to be directed to either the CPU or counter
void Scic::setNMItoCPU(bool value)
{
    NMItoCPU = value;
}

// Increase Stage2 Counter, Stage1 counter is not required
// as psion does not access to this counter.
// The purpose of a Stage1 counter would be to count in ms
void Scic::counterInc()
{
    stage2++;
    if (stage2 >= 0x1000) stage2 = 0;
    acout = (stage2 >= 0x0800);
}

void Scic::counterReset()
{
    stage2 = 0;
}


// Read on line Port 5. Returns current acout, pulse and
// keyboard state taking in to account current counter value.
BYTE Scic::R_port5()
{
    BYTE port5 = keypad->activeKeyLines(stage2);
    if (acout || pulse) port5 |= 0x02;
    return port5;
}

// Main SCIC loop to maintain timings and check for acout
bool Scic::doIteration(int cycles, bool powered)
{
    nextNMI -= cycles;
    if (nextNMI < 0)
    {
        nextNMI += 921600;
        if (NMItoCPU) nextNMI++; //cpu->notify_NMI();
        else counterInc();
    }

    if (acout) return true;
    else if (keypad->isOnClearPressed()) return true;

    return powered;
}
