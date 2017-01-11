#ifndef EMUCORE_H
#define EMUCORE_H

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "global.h"

#define FILEIO_ROM_Unknown			0x00
#define FILEIO_ROM_CM				0x01
#define FILEIO_ROM_XP				0x02
#define FILEIO_ROM_LA				0x03
#define FILEIO_ROM_MLA				0x04
#define FILEIO_ROM_POS200			0x05
#define FILEIO_ROM_POS250			0x06
#define FILEIO_ROM_POS350			0x07
#define FILEIO_ROM_LZ				0x08
#define FILEIO_ROM_LZ64				0x09
#define FILEIO_ROM_POS432			0x0a
#define FILEIO_ROM_POS464			0x0b
#define FILEIO_ROM_POS296			0x0c
#define FILEIO_SUCCESS				0xfc
#define FILEIO_RAM_SaveFail			0xfd
#define FILEIO_ROM_Invalid			0xfe
#define FILEIO_ROM_LoadFail			0xff

#define SYNC_FRAMES                 50
#define SYNC_CYCLES                 46080
#define VSYNC_RATE                  20

typedef struct {
    ADDRESS ramLow;
    ADDRESS ramHigh;
    int maxRomBanks;
    int maxRamBanks;
    int lcd;
    bool numKB;
    int ramSizeKB;
    char name[50];
} STRUCT_settings;

class EmuCore
{
private:
	void* windowPointer;

	bool power;
	int currentMode;
	bool romLoaded;

	std::string romPath;
	std::string ramPath;

	std::thread coreThread;
	std::mutex coreMutex;
	std::mutex pausedMutex;
	std::condition_variable pausedWait;
	std::atomic<bool> killCore;
	std::atomic<bool> coreAlive;
	std::atomic<bool> paused;
	std::atomic<int> emuSpeed;

	void coreRountine();

	void INTERNAL_reset();
	void INTERNAL_setPower(bool hasPower);
	//int save();

public:
    explicit EmuCore(void* ptrWindow);
    ~EmuCore();
	void startCore();
	void stopCore();
	void pause(bool doPause);

    bool isPowered();
    void setPower(bool power);
    void hardReset();

	int getEmuSpeed();

    int load(std::string filename);
    int analyseROM(std::string filename);
	int save();
};

extern EmuCore *emucore;

#endif // CORE_H
