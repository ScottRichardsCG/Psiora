#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include "global.h"

// Switch debugging on and off in the "global.h" header
// DEBUG_MODE_ENABLED
// DEBUG_MODE_ENABLED_CPU

#ifdef DEBUG_MODE_ENABLED
class Debug
{
private:
    bool logOpen;
    FILE *logFile;
#ifdef DEBUG_MODE_ENABLED_CPU
	FILE *logFileCPU;
    int ignoreCmd;
    int breakPoint;
#endif
public:
    Debug();
    ~Debug();
    void writeLog(char *log);
    void writeOutput(char *log);
#ifdef DEBUG_MODE_ENABLED_CPU
    void writeCPU_1_0(ADDRESS pc, BYTE op);
    void writeCPU_1_8(ADDRESS pc, BYTE op, BYTE operand);
    void writeCPU_1_16(ADDRESS pc, BYTE op, WORD_16 operand);
    void writeCPU_2(WORD_16 D, WORD_16 X, ADDRESS SP, BYTE CCR);
#endif

};

extern Debug *debug;
#endif

#endif // DEBUG_H
