#include "global.h"
#include "emucore.h"
#include "renderer.h"
#include "keypad.h"
#include "memory.h"
#include "scic.h"
#include "cpu.h"
#include "lcd.h"
#include "debug.h"
#include <QFile>
#include <QString>
#include <QElapsedTimer>
#include <iostream>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

EmuCore *emucore;

const STRUCT_settings settings[13] = {
    { 0x0400, 0x0400, 2, 2, 0, false, 0, "Invalid ROM" },	// Invalid
    { 0x2000, 0x3fff, 2, 2, 0, false, 8, "CM" },            // CM
    { 0x2000, 0x5fff, 2, 2, 0, false, 16, "XP" },           // XP
    { 0x0400, 0x7fff, 2, 2, 0, false, 32, "LA" },           // LA
    { 0x0400, 0x7fff, 4, 2, 0, false, 32, "MLA" },          // MLA
    { 0x2000, 0x5fff, 2, 2, 0, false, 16, "POS200" },       // Pos200 -- Pos200 XP?
    { 0x0400, 0x7fff, 2, 2, 0, true, 32, "POS250" },        // Pos250 -- Pos200 LA?
    { 0x0400, 0x7fff, 6, 2, 0, false, 96, "POS350" },       // Pos350
    { 0x0400, 0x7fff, 4, 2, 1, false, 32, "LZ" },           // LZ
    { 0x0400, 0x7fff, 4, 4, 1, false, 64, "LZ64" },         // LP (LZ64)
    { 0x0400, 0x7fff, 2, 4, 1, false, 32, "POS432" },       // Pos432
    { 0x0400, 0x7fff, 4, 4, 1, false, 64, "POS464" },       // Pos464
    { 0x0400, 0x7fff, 6, 2, 0, true, 96, "Pos296" }         // Pos296
};

// ---------------------------------
// | CONSTRUCTION & DECONSTRUCTION |
// ---------------------------------
EmuCore::EmuCore(void *ptrWindow)
{
    windowPointer = ptrWindow;
    killCore = false;
}

EmuCore::~EmuCore() {
}

// -------------------------
// | MAIN EMULATION THREAD |
// -------------------------
void EmuCore::run() {
#ifdef Q_OS_WIN
    timeBeginPeriod(1);
#endif
    power = false;
    fileio.romLoaded = false;

#ifdef DEBUG_MODE_ENABLED
    debug = new Debug();
#endif
    renderer = new Renderer(windowPointer);
    lcd = new Lcd();
    keypad = new Keypad();
    scic = new Scic();
    memory = new Memory();
    cpu = new Cpu();

    std::cerr << "Thread Running\n";

    // Main Loop
    /* int a = 0;
    while (!killCore)
    {
        a++;
        std::cerr << "Thread " << a << std::endl;
        QThread::currentThread()->sleep(1);
        // some Code

    } */

    // Emulation Loop
    int busCycles = 0;
    int cursorCycles = 0;
    bool tmpPower = false;
    QElapsedTimer timer;
    timer.start();
    int resync = 46080;

    while (!killCore) {
        lockCore();
        busCycles = cpu->doIteration();
        tmpPower = scic->doIteration(busCycles, power);
        if (tmpPower != power) {
            INTERNAL_setPower(tmpPower);
        }

        cursorCycles += busCycles;
        if (cursorCycles >= 46080) {
            cursorCycles -= 46080;
            lcd->update(true);
        }
        unlockCore();

        resync -= busCycles;
        if (resync <= 0) {
            qint64 remaining = SYNC_FRAMES - timer.elapsed();
            if ((remaining < 0) || (remaining > SYNC_FRAMES)) {
                // Error in timing, maybe overflow.
                // Timing invalid and will be ignored
                resync = SYNC_CYCLES;
            } else if (remaining < 10) {
                // Can't always get sleep accuracy lower than 10ms
                // so accumulate time into next cycle
                resync += SYNC_CYCLES;
            } else {
                // Sleep thread
                QThread::currentThread()->msleep(remaining);
                resync = SYNC_CYCLES;
            }
            timer.restart();
        }

    }

    //save();

    delete cpu;
    delete memory;
    delete keypad;
    delete scic;
    delete lcd;
    delete renderer;
#ifdef DEBUG_MODE_ENABLED
    delete debug;
#endif

#ifdef Q_OS_WIN
    timeEndPeriod(1);
#endif

    std::cerr << "Thread Quit\n";
}

// ------------------
// | THREAD CONTROL |
// ------------------

void EmuCore::halt() {
    killCore = true;
    emucore->wait();
}

void EmuCore::lockCore() {
    mutex.lock();
}

void EmuCore::unlockCore() {
    mutex.unlock();
}

// -----------------------------------
// |      INTERNAL FUNCTIONALITY     |
// |        NOT THREAD SAFE          |
// | ONLY CALLED BY EMULATION THREAD |
// -----------------------------------

void EmuCore::INTERNAL_reset() {
    power = false;
    lcd->setPower(false);
    memory->resetBanks();
    cpu->setPower(false);
    scic->init();
    memory->clearRAM();
}

void EmuCore::INTERNAL_setPower(bool value) {
    if (value == power)
        return;
    power = value;
    lcd->setPower(power);
    if (power)
        scic->counterInc();
    memory->resetBanks();
    cpu->setPower(power);
}

// ------------------------------------
// |      EXTERNAL FUNCTIONALITY      |
// |      THREAD SAFE USING MUTEX     |
// ------------------------------------

bool EmuCore::isPowered() { return power; }

void EmuCore::setPower(bool value) {
    lockCore();
    INTERNAL_setPower(value);
    unlockCore();
}

int EmuCore::load(QString filename) {
    int romType;
    romType = analyseROM(filename);
    if (romType == FILEIO_ROM_LoadFail) return romType;
    if (romType == FILEIO_ROM_Invalid) return romType;

    lockCore();

    lcd->setMode(settings[romType].lcd);
    memory->settings(settings[romType].ramLow,
                     settings[romType].ramHigh,
                     settings[romType].maxRamBanks,
                     settings[romType].maxRomBanks);
    currentMode = romType;

    fileio.romPath = filename;
    fileio.ramPath = filename + ".sav";

    INTERNAL_reset();
    memory->clearROM();
    memory->clearRAM();

    QFile file;
    QByteArray data;

    file.setFileName(fileio.romPath);
    if (!file.open(QIODevice::ReadOnly)) {
        unlockCore();
        return FILEIO_ROM_LoadFail;
    }
    data = file.readAll();
    file.close();
    memory->writeROM(data.data(), data.size());
    fileio.romLoaded = true;

    file.setFileName(fileio.ramPath);
    if (file.open(QIODevice::ReadOnly))
    {
        data = file.readAll();
        file.close();
        memory->writeRAM(data.data(), data.size());
    }

    unlockCore();
    return romType;
}

int EmuCore::save() {
    if (!fileio.romLoaded) return FILEIO_SUCCESS;

    QFile file(fileio.ramPath);
    if (!file.open(QIODevice::ReadWrite)) {
        return FILEIO_RAM_SaveFail;
    }
    char ram[983040];
    unsigned int len = settings[currentMode].maxRamBanks * 0x4000;
    memory->readRAM(ram, len);
    file.write(QByteArray(ram, len));
    file.close();
    return FILEIO_SUCCESS;
}

int EmuCore::analyseROM(QString filename) {
    int preset;
    BYTE modeByte;
    QFile file;
    QByteArray romData;

    file.setFileName(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return FILEIO_ROM_LoadFail;
    }
    romData = file.readAll();
    file.close();
    if (romData.size() != 32768 && romData.size() != 65536) {
        return FILEIO_ROM_Invalid;
    }

    modeByte = romData.data()[0x7fe8];
    switch (modeByte) {
    case 0x00:  preset = FILEIO_ROM_CM;       break;
    case 0x01:  preset = FILEIO_ROM_XP;       break;
    case 0x02:
        if (romData.size() == 32768)
            preset = FILEIO_ROM_LA;
        else
            preset = FILEIO_ROM_MLA;
        break;
    case 0x0e:  preset = FILEIO_ROM_LZ;       break;
    case 0x0d:  preset = FILEIO_ROM_LZ64;     break;
    case 0x81:  preset = FILEIO_ROM_POS200;   break;
    case 0x82:  preset = FILEIO_ROM_POS250;   break;
    case 0x14:  preset = FILEIO_ROM_POS350;   break;
    case 0x1e:  preset = FILEIO_ROM_POS432;   break;
    case 0x1d:  preset = FILEIO_ROM_POS464;   break;
    default:    preset = FILEIO_ROM_Unknown;  break;
    }
    return preset;
}


void EmuCore::hardReset() {
    lockCore();
    INTERNAL_reset();
    unlockCore();
}
