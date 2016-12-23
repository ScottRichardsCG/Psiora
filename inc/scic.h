#ifndef SCIC_H
#define SCIC_H

#include "global.h"

class Scic
{
private:
    bool NMItoCPU;
    int nextNMI;
    bool acout;
    bool pulse;
    bool alarm;
    unsigned int stage2;
public:
    void init();
    void setAlarm(bool value);
    void setPulse(bool value);
    void setNMItoCPU(bool value);
    void counterInc();
    void counterReset();
    BYTE R_port5();
    bool doIteration(int cycles, bool powered);
};

extern Scic *scic;

#endif // SCIC_H
