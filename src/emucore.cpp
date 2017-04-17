#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <string>
#include <fstream>
#include <iostream>

#include "global.h"
#include "emucore.h"
#include "renderer.h"
#include "keypad.h"
#include "memory.h"
#include "scic.h"
#include "cpu.h"
#include "lcd.h"
#include "datapak.h"
#include "debug.h"

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
    { 0x0400, 0x7fff, 2, 6, 0, false, 96, "POS350" },       // Pos350
    { 0x0400, 0x7fff, 4, 2, 1, false, 32, "LZ" },           // LZ
    { 0x0400, 0x7fff, 4, 4, 1, false, 64, "LZ64" },         // LP (LZ64)
    { 0x0400, 0x7fff, 2, 4, 1, false, 32, "POS432" },       // Pos432
    { 0x0400, 0x7fff, 4, 4, 1, false, 64, "POS464" },       // Pos464
    { 0x0400, 0x7fff, 2, 6, 0, true, 96, "Pos296" }         // Pos296
};

// ptrWindow contacts a pointer to valid X11 or HWND window object.
EmuCore::EmuCore(void *ptrWindow)
{
    windowPointer = ptrWindow;

#ifdef DEBUG_MODE_ENABLED
    debug = new Debug();
#endif
    renderer = new Renderer(windowPointer);
    lcd = new Lcd();
    keypad = new Keypad();
    scic = new Scic();
	datapak = new Datapak();
    memory = new Memory();
    cpu = new Cpu();

}

// Make sure thread has been stopped before we deconstruct object
EmuCore::~EmuCore() {
	if (coreThread.joinable()) {
		stopCore();
	}

    delete cpu;
    delete memory;
    delete keypad;
    delete scic;
	delete datapak;
    delete lcd;
    delete renderer;
#ifdef DEBUG_MODE_ENABLED
    delete debug;
#endif
}

// Core thread - Note: Run as a seperate thread
void EmuCore::coreRountine() {

	std::unique_lock<std::mutex> lk(coreMutex);

#ifdef Q_OS_WIN
	// Increases timing accuracy for windows platform in this thread
	timeBeginPeriod(1);
#endif
    // Emulation Loop
	paused = false;
	power = false;
	romLoaded = false;
	coreAlive = true;
	killCore = false;
	emuSpeed = 0;
    int busCycles = 0;
    int cursorCycles = 0;
    bool tmpPower = false;
    int nextSync = 46080;
	unsigned int emuCyclesPerSecond = 0;

	lk.unlock();

	std::chrono::high_resolution_clock::time_point syncTimerStart = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point emuSpeedTimerStart = std::chrono::high_resolution_clock::now();

	while (!killCore) {
		if (paused) {
			// Paused so wait for a unpause signal to save resources
			std::unique_lock<std::mutex> lk(pausedMutex);
			pausedWait.wait(lk);
			lk.unlock();
		}
		else {
			if (power == false) {
				// Power is off so just wait for a second and then check wakeups (poweron/acout)
				// Also emmit NMI signals for counterStage2 in scic
				std::this_thread::sleep_for(std::chrono::seconds(1));
				lk.lock();
				emuSpeed = 100;
				tmpPower = scic->doIteration(921600, power);
				if (tmpPower != power) {
					INTERNAL_setPower(tmpPower);
				}
				lk.unlock();
			}
			else if (power == true) {
				lk.lock();
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

				lk.unlock();

				std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

				nextSync -= busCycles;
				if (nextSync <= 0) {
					std::chrono::milliseconds elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(end - syncTimerStart);
					if ((elapsedTime.count() < 0) || (elapsedTime.count() > 50)) {
						// Error in timing, maybe overflow.
						// Timing invalid and will be ignored
						nextSync = SYNC_CYCLES;
					}
					else if (elapsedTime.count() >= 45) {
						// Can't always get sleep accuracy lower enough for this
						// so accumulate time into next cycle
						nextSync += SYNC_CYCLES;
					}
					else {
						// Sleep thread
						nextSync = SYNC_CYCLES;
						std::this_thread::sleep_for((std::chrono::milliseconds(50) - elapsedTime));
					}
					syncTimerStart = std::chrono::high_resolution_clock::now();
				}

				// Calculate Emulation Speed every second
				emuCyclesPerSecond += busCycles;
				std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - emuSpeedTimerStart);
				if (elapsed.count() >= 1000) {
					emuSpeed = emuCyclesPerSecond / 9216;
					emuSpeedTimerStart = std::chrono::high_resolution_clock::now();
					emuCyclesPerSecond = 0;
				}
			}
		}
    }

	lk.lock();

	coreAlive = false; // Set this to false to tell functions the objects should not be accessed

	lk.unlock();

#ifdef Q_OS_WIN
	// Restore timing accuracies to original
    timeEndPeriod(1);
#endif

	return;
}

void EmuCore::startCore() {
	if (coreThread.joinable())
		return; // Thread already running

	coreThread = std::thread(&EmuCore::coreRountine, this);
}

void EmuCore::stopCore() {
	if (!coreThread.joinable())
		return; // Thread not running

	killCore = true;

	if (paused) {
		// Core paused so we need to wake it so it can clean up
		pausedWait.notify_all();
	}

	coreThread.join();
}

void EmuCore::pause(bool doPause) {
	if (paused == doPause)
		return;

	paused = doPause;
	if (!paused)
		pausedWait.notify_all();
}

// NOT THREAD SAFE
// This is only called by the core thread to reset emulation state
void EmuCore::INTERNAL_reset() {
    power = false;
	lcd->init();
    memory->resetBanks();
	cpu->setPower(false);
	scic->init();
    memory->clearRAM();
	memory->resetBanks();
}

// NOT THREAD SAFE
// This is only called by the core thread to switch virtual devices on and off
void EmuCore::INTERNAL_setPower(bool value) {
    if (value == power)
        return;
    power = value;
    lcd->setPower(power);
	scic->setPower(power);
	memory->setPower(power);
    cpu->setPower(power);
}

// ---------------------------------------------------
//  PUBLIC API - Everything below must be thread safe
// ---------------------------------------------------

bool EmuCore::isPowered() {
	std::lock_guard<std::mutex> lk(coreMutex);
	return power;
	// Lock released as out of scope
}

void EmuCore::setPower(bool hasPower) {
	std::lock_guard<std::mutex> lk(coreMutex);
	INTERNAL_setPower(hasPower);
	// Lock released as out of scope
}

int EmuCore::load(std::string filename) {
	// Check if the ROM is valid and detect its emulation settings
	int romType = analyseROM(filename);
	if (romType == FILEIO_ROM_LoadFail || romType == FILEIO_ROM_Invalid)
		return romType;

	// Load ROM and RAM data before we apply them to the emulator
	// If there is an error, we can abort before emulation data is changed or unsaved data lost

	char romTmp[65536] = { 0 };
	char ramTmp[98304] = { 0 };
	std::ifstream romStream, ramStream;

	romStream.open(filename, std::ios_base::in | std::ios_base::binary);
	if (!romStream.good())
		return FILEIO_ROM_LoadFail;
	romStream.read(romTmp, 65536);
	romStream.close();

	// NB: if error opening RAM we just continue with a fresh empty RAM
	ramStream.open(filename + ".sav", std::ios_base::in | std::ios_base::binary);
	if (ramStream.good()) {
		ramStream.read(ramTmp, 98304);
		ramStream.close();
	}

	std::lock_guard<std::mutex> lock(coreMutex);

	lcd->setMode(settings[romType].lcd);
	memory->settings(settings[romType].ramLow,
		settings[romType].ramHigh,
		settings[romType].maxRomBanks,
		settings[romType].maxRamBanks);
	currentMode = romType;

	romPath = filename;
	ramPath = filename + ".sav";
	romLoaded = true;

	INTERNAL_reset();
	memory->clearROM();
	memory->clearRAM();

	memory->writeROM(romTmp, 65536);
	memory->writeRAM(ramTmp, 98304);

	return romType;
}


int EmuCore::save() {
    if (!romLoaded || !power)
		return FILEIO_SUCCESS;
	if (power)
		return FILEIO_SaveFail_PowerOn;

	std::ofstream ramStream;
	ramStream.open(ramPath, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
	if (!ramStream.good())
		return FILEIO_RAM_SaveFail;

	unsigned int length = settings[currentMode].maxRamBanks * 0x4000;
	char ram[98304];
	memory->readRAM(ram, length);
	ramStream.write(ram, length);
	ramStream.close();

	return FILEIO_SUCCESS;
}

// This function analyses a ROM file and returns its intended hardware setup
int EmuCore::analyseROM(std::string filename) {
	int preset, romSize;
	std::ifstream romStream;
	romStream.open(filename, std::ios_base::in | std::ios_base::binary);
	if (!romStream.good())
		return FILEIO_ROM_LoadFail;

	romStream.seekg(0, std::ios_base::end);
	romSize = romStream.tellg();
	if (romSize != 32768 && romSize != 65536)
		return FILEIO_ROM_Invalid;

	romStream.seekg(0x7FE8);

	BYTE modeByte = romStream.get();
	romStream.close();

    switch (modeByte) {
    case 0x00:  preset = FILEIO_ROM_CM;       break;
    case 0x01:  preset = FILEIO_ROM_XP;       break;
    case 0x02:
        if (romSize == 32768)
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
	std::lock_guard<std::mutex> lk(coreMutex);
    INTERNAL_reset();
	INTERNAL_setPower(true);
	// Lock released when out of scope
}

int EmuCore::getEmuSpeed() {
	// emuSpeed is cloned otherwise a write could change the value during these 3 read operations
	int val = emuSpeed;
	return val < 0 ? 0 : val > 100 ? 100 : val;
}


// Public Datapak load/create/eject functions - thread safe
int EmuCore::ejectPak(int id, bool force) {
	std::lock_guard<std::mutex> lk(coreMutex);
	return datapak->eject(id, force);
}

int EmuCore::insertPak(int id, std::string filename) {
	std::lock_guard<std::mutex> lk(coreMutex);
	return datapak->load(id, filename);
}

int EmuCore::createPak(bool ram, int siz, std::string filename) {
	std::lock_guard<std::mutex> lk(coreMutex);
	return datapak->create (ram, siz, filename);
}