#ifndef MEMORY_H
#define MEMORY_H

#include "global.h"

class Memory
{
private:
    struct {
        BYTE data[98304];
        int page;
        int maxPage;
        int lowRam;
        int highRam;
    } ram;

    struct {
        BYTE data[65536];
        int page;
        int maxPage;
    } rom;

    void nextRamBank();
    void nextRomBank();
    BYTE readCPU(ADDRESS address);
    void writeCPU(ADDRESS address, BYTE data);
    BYTE readLCD(ADDRESS address);
    void writeLCD(ADDRESS address, BYTE data);
    void triggerHardware(ADDRESS address);

public:
    Memory();
    void resetBanks();
	void setPower(bool power);
    void settings(ADDRESS ramLow, ADDRESS ramHigh, int maxRomBanks, int maxRamBanks);

    void clearRAM();
    void writeRAM(char *bin, int size);
    void readRAM(char *bin, int size);
    void clearROM();
    void writeROM(char *bin, int size);
    void readROM(char *bin, int size);

    BYTE read(ADDRESS address);
    WORD_16 read16(ADDRESS address);
    void write(ADDRESS address, BYTE data);
    void write16(ADDRESS address, WORD_16 data);

    BYTE readDirect(ADDRESS address);
    WORD_16 readDirect16(ADDRESS address);
    void writeDirect(ADDRESS address, BYTE data);
    void writeDirect16(ADDRESS address, WORD_16 data);

    ADDRESS readAddress(ADDRESS address);
    void writeAddress(ADDRESS address, ADDRESS data);

    BYTE getCapsLock();
    void setCapsLock(BYTE data);

};

extern Memory *memory;

#endif // MEMORY_H
