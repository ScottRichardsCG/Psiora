#include "memory.h"
#include "emucore.h"
#include "scic.h"
#include "cpu.h"
#include "lcd.h"
#include <iostream>

Memory *memory;

Memory::Memory()
{
	clearRAM();
	clearROM();
	resetBanks();
}

void Memory::settings(ADDRESS ramLow, ADDRESS ramHigh, int maxRomBanks, int maxRamBanks)
{
    ram.lowRam = ramLow;
    ram.highRam = ramHigh;
    ram.maxPage = maxRamBanks;
    rom.maxPage = maxRomBanks;
}

void Memory::clearRAM() { memset(ram.data, 0x00, 98304); }
void Memory::writeRAM(char *bin, int size) { memcpy(ram.data, bin, size); }
void Memory::readRAM(char *bin, int size) { memcpy(bin, rom.data, size); }

void Memory::clearROM() { memset(rom.data, 0x00, 65536); }
void Memory::writeROM(char *bin, int size) { memcpy(rom.data, bin, size); }
void Memory::readROM(char *bin, int size) { memcpy(bin, rom.data, size); }

void Memory::setPower(bool power) {
    if (power == false) {
		resetBanks();
	}
}

// Control current RAM and ROM banks
void Memory::resetBanks()
{
    ram.page = 0;
    rom.page = 0;
}

void Memory::nextRamBank()
{
    ram.page++;
    if (ram.page >= (ram.maxPage-1))
    {
        ram.page = 0;
    }
}

void Memory::nextRomBank()
{
    rom.page++;
    if (rom.page >= (rom.maxPage-1))
    {
        rom.page = 0;
    }
}

// Read from the CPU Registers
BYTE Memory::readCPU(ADDRESS address)
{
    switch (address)
    {
    case 0x0002: return 0;                      /// Port1 - Low Addr Bus - DoNotUse
    //case 0x0003: return datapak->R_port2();
    case 0x0006: return 0;                      /// Port3 - Data Bus - DoNotUse
    case 0x0007: return 0;                      /// Port4 - High Addr Bus - DoNotUse
    case 0x0008: return ram.data[0x0008];
    case 0x0009: return cpu->R_timer1_FRC_H();
    case 0x000a: return ram.data[0x000a];
    case 0x000b: return ram.data[0x000b];
    case 0x000c: return ram.data[0x000c];
    case 0x000d: return 0;                      /// Input capture register???
    case 0x000e: return 0;                      /// Input capture register???
    case 0x000f: return ram.data[0x000f];
    case 0x0010: return ram.data[0x0010];
    case 0x0011: return 0x20;                   /// Tx/Rx Control Status Register
    case 0x0012: return 0;                      /// Receive data register
    case 0x0014: return ram.data[0x0014];
    case 0x0015: return scic->R_port5();
    //case 0x0017: return datapak->R_port6();
    case 0x0018: return 0;                      /// Port7
    case 0x0019: return 0xff;                   /// OCR???
    case 0x001a: return 0xff;                   /// OCR???
    case 0x001b: return 0x20;                   /// Timer3 Control/Status Register
    case 0x001d: return 0;                      /// Timer2 Up Counter
    case 0x001f: return 0;                      /// Test Register - DoNotUse
    }
    return 0;
}

// Write to CPU Registers
void Memory::writeCPU(ADDRESS address, BYTE data)
{
    switch (address)
    {
    //case 0x0001: datapak->W_port2DDR(data); break;
    //case 0x0003: datapak->W_port2(data);    break;
    case 0x0008: cpu->W_timer1_CSR(data);   break;
    case 0x0009: cpu->W_timer1_FRC_H(data); break;
    case 0x000a: cpu->W_timer1_FRC_L(data); break;
    case 0x000b:
        ram.data[0x000b] = data;
        cpu->notify_timer1_OCR();
        break;
    case 0x000c:
        ram.data[0x000c] = data;
        cpu->notify_timer1_OCR();
        break;
    case 0x000f: cpu->W_timer2_CSR(data);   break;
    case 0x0010: ram.data[0x0010] = data;   break;
    case 0x0014: ram.data[0x0014] = data;   break;
    //case 0x0016: datapak->W_port6DDR(data); break;
    //case 0x0017: datapak->W_port6(data);    break;
    }
}

// Read from LCD device
BYTE Memory::readLCD(ADDRESS address)
{
    if ((address & 65504) == 0x0180)
    {
        if (address & 1)
            return lcd->R_data();
        else
            return lcd->R_control();
    }
    return 0;
}

// Write to LCD device
void Memory::writeLCD(ADDRESS address, BYTE data)
{
    if ((address & 65504) == 0x0180)
    {
        if (address & 1)
            lcd->W_data(data);
        else
            lcd->W_control(data);
    }
}

// Access hardware triggers
void Memory::triggerHardware(ADDRESS address)
{
    switch (address & 65504)
    {
	case 0x01c0: scic->switchOff();						break;
    case 0x0200: scic->setPulse(true);					break;
    case 0x0240: scic->setPulse(false);					break;
    case 0x0280: break; //datapak->setV21(true);		break;
    case 0x02c0: break; //datapak->setV21(false);		break;
    case 0x0300: scic->counterReset();					break;
    case 0x0340: scic->counterInc();					break;
    case 0x0360: resetBanks();      					break;
    case 0x0380: scic->setNMItoCPU(true);			    break;
    case 0x03a0: nextRamBank();							break;
    case 0x03c0: scic->setNMItoCPU(false);				break;
    case 0x03e0: nextRomBank();						    break;
    }
}

// Read a single byte from RAM or ROM directly
BYTE Memory::readDirect(ADDRESS address)
{
    if (address <= 0x00ff)
        return ram.data[address];
    else if (address >= ram.lowRam && address <= 0x3fff)
        return ram.data[address];
    else if (address >= 0x4000 && address <= ram.highRam)
        return ram.data[address + (ram.page << 14)];
    else if (address >= 0x8000 && address <= 0xbfff)
    {
        switch (rom.page)
        {
        case 0: return rom.data[address - 0x8000];
        case 1: return rom.data[address];
        case 2: return rom.data[address + 0x4000];
        }
    }
    else if (address >= 0xc000 && address <= 0xffff)
        return rom.data[address - 0x8000];
    return 0;
}

// Read a single word (2 bytes) from RAM or ROM directly
WORD_16 Memory::readDirect16(ADDRESS address)
{
    WORD_16 tmp;
    tmp.b.h = readDirect(address);
    tmp.b.l = readDirect(address + 1);
    return tmp;
}

// Write a single byte to RAM directly
void Memory::writeDirect(ADDRESS address, BYTE data)
{
    if (address <= 0x00ff)
        ram.data[address] = data;
    else if ((address >= ram.lowRam) && (address <= 0x3fff))
        ram.data[address] = data;
    else if ((address >= 0x4000) && (address <= ram.highRam))
        ram.data[address + (ram.page << 14)] = data;
}

// Write a single word (2 bytes) to RAM directly
void Memory::writeDirect16(ADDRESS address, WORD_16 data)
{
    writeDirect(address, data.b.h);
    writeDirect(address + 1, data.b.l);
}

// Read a byte from the desired address.
BYTE Memory::read(ADDRESS address)
{
    if (address <= 0x3f)
        return readCPU(address);
    else if (address >= 0x0180 && address <= 0x01bf)
        return readLCD(address);
    else if (address >= 0x01c0 && address <= 0x03ff)
        triggerHardware(address);
    else
        return readDirect(address);

    return 0;
}

// Read a word (2 consecutive bytes) from the desired address.
WORD_16 Memory::read16(ADDRESS address)
{
    WORD_16 tmp;
    tmp.b.h = read(address);
    tmp.b.l = read(address + 1);
    return tmp;
}

// Write a byte to a desired address.
void Memory::write(ADDRESS address, BYTE data)
{
    if (address <= 0x3f)
        writeCPU(address, data);
    else if (address >= 0x0180 && address <= 0x01bf)
        writeLCD(address, data);
    else if (address >= 0x01c0 && address <= 0x03ff)
        triggerHardware(address);
    else
        writeDirect(address, data);
}

// Write a word (2 consecutive bytes) to the desired address.
void Memory::write16(ADDRESS address, WORD_16 data)
{
    write(address, data.b.h);
    write(address + 1, data.b.l);
}

ADDRESS Memory::readAddress(ADDRESS address)
{
    WORD_16 tmp = memory->read16(address);
    return tmp.w;
}

void Memory::writeAddress(ADDRESS address, ADDRESS data)
{
    WORD_16 tmp;
    tmp.w = data;
    memory->write16(address, tmp);
}
