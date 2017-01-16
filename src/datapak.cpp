#include <fstream>
#include <string>
#include <cstring>

#include "datapak.h"

#define pakAddr(id)         ((pak[id].addr.segment << 14) + (pak[id].addr.page << 8) + pak[id].addr.counter)
#define pakData(id)         pak[id].data[pakAddr(id)]

Datapak *datapak;

Datapak::Datapak()
{
	svpp = 5;
	pak[0].properties.inserted = false;
	pak[1].properties.inserted = false;
	pak[2].properties.inserted = false;
}

// Eject pak from Emulator and save to file
bool Datapak::eject(int id, bool force) {
	bool ret = save(id);
	if (force == false && ret == false) {
		// Something went wrong saving, so return error
		return false;
	}
	// Otherwise it went fine, or we have been told to ignore save fail
	pak[id].properties.inserted = false;
	return ret;
}

// Save pak image to file
bool Datapak::save(int id) {
	if (!pak[id].properties.inserted)
		return true;		// Nothing to do, so return successful
	if (pak[id].properties.readOnly)
		return true;		// Pak is readonly, so we dont want to save. Return successful

	BYTE pakData[MAX_PAK_SIZE + 6];
	// Rampaks format to 0x00, Datapaks format to 0xff
	memset(pakData, (pak[id].properties.rampak ? 0x00 : 0xff), MAX_PAK_SIZE + 6);

	pakData[0] = 'O';
	pakData[1] = 'P';
	pakData[2] = 'K';
	
	memcpy(&pakData[6], pak[id].data, pak[id].siz);

	// Scan from end of pak to find last data byte
	// none 0x00 byte on a rampak, or none 0xff byte on a datapak
	int newSize;
	for (newSize = (pak[id].siz + 6); newSize >= 0; newSize--) {
		if (pak[id].data[newSize] != (pak[id].properties.rampak ? 0x00 : 0xff)) {
			break;
		}
	}
	if (newSize <= 6) {
		// This should never occur, but if it does, save entire pak uncompressed
		newSize = pak[id].siz + 6;
	}

	pakData[3] = ((newSize - 1) >> 16) & 0xff;
	pakData[4] = ((newSize - 1) >> 8) & 0xff;
	pakData[5] = (newSize - 1) & 0xff;

	std::ofstream savFile;
	savFile.open(pak[id].filename, std::ios_base::binary | std::ios_base::trunc | std::ios_base::out);
	if (!savFile.good())
		return false;
	
	savFile.write((char*)pakData, newSize);
	savFile.close();
	return true;
}

// Create a new blank ram/datapak image and save it to file
bool Datapak::create(bool ram, unsigned int siz, std::string filename) {
	BYTE blankPak[MAX_PAK_SIZE + 6];
	// Rampaks format to 0x00, Datapaks format to 0xff
	memset(blankPak, (ram ? 0x00 : 0xff), siz + 6);
	blankPak[0] = 'O';
	blankPak[1] = 'P';
	blankPak[2] = 'K';
	blankPak[3] = (siz >> 16) & 0xff;
	blankPak[4] = (siz >> 8) & 0xff;
	blankPak[5] = siz & 0xff;

	if (ram)
		blankPak[6] |= 0x02;	// Set rampak flag
	if (siz >= 0x8000)
		blankPak[6] |= 0x04;	// Paks more than 32KB are paged by default, set flag for this
	blankPak[7] = siz >> 13;	// Set number of 8KB pages in pak

	std::ofstream savFile;
	savFile.open(filename, std::ios_base::binary | std::ios_base::trunc | std::ios_base::out);
	if (!savFile.good())
		return false;
	savFile.write((char *)blankPak, siz + 6);
	savFile.close();
	return true;
}

// Load pak from an OPK image
int Datapak::load(int id, std::string filename) {
	pak[id].properties.inserted = false;
	BYTE loadData[MAX_PAK_SIZE + 6];

	std::ifstream loadFile;
	loadFile.open(filename, std::ios_base::binary | std::ios_base::in);
	if (!loadFile.good())
		return PAK_ERR_CANT_OPEN;
	loadFile.read((char *)loadData, MAX_PAK_SIZE + 6);
	loadFile.close();

	if (loadData[0] != 'O' || loadData[1] != 'P' || loadData[2] != 'K')
		return PAK_ERR_NOT_OPK;

	memset(pak[id].data, (loadData[6] ? 0xff : 0x00), MAX_PAK_SIZE);

	unsigned int size;
	size = (loadData[3] << 16) + (loadData[4] << 8) + loadData[5] + 1;
	if ((loadData[7] << 13) >= MAX_PAK_SIZE)
		return PAK_ERR_TOO_LARGE;

	memcpy(pak[id].data, &loadData[6], size);
	pak[id].filename = filename;
	
	pak[id].siz = pak[id].data[1] << 13;
	pak[id].addr.page = 0;
	pak[id].addr.counter = 0;
	pak[id].addr.segment = 0;
	pak[id].properties.rampak = !(pak[id].data[0] & 0x02);
	
	if (pak[id].data[0] & 0x04) {
		// Paged or Segmented pak
		pak[id].properties.paged = true;
		pak[id].addr.page_mask = (pak[id].siz >> 8) - 1;
		pak[id].addr.counter_mask = 0xff;
	}
	else {
		// Linear pak
		pak[id].properties.paged = false;
		pak[id].addr.page_mask = 0x00;
		pak[id].addr.counter_mask = pak[id].siz - 1;
	}

	// Set to 0, as segmented packs not yet implemented
	pak[id].addr.segment = 0;
	pak[id].addr.segment_mask = 0;

	return PAK_SUCCESSFUL;
}

// Change pak voltage (5 volts to read, 21 volts to write)
void Datapak::setVoltage21(bool volt) {
	svpp = volt ? 21 : 5;
}

// Port 2 - Data port
BYTE Datapak::R_port2() {
	return port2.latch;
}

void Datapak::W_port2(BYTE data) {
	port2.latch = data;
	// If output enabled (pulled low), process data to pak
	if (!port6.oe)
		processDataOnPort();
}

void Datapak::W_port2DDR(BYTE data) {
	// If any lines changed from input to output, process data to pak
	if (port2.ddr != data) {
		port2.ddr = data;
		processDataOnPort();
	}
}

// Port6 - Control port
BYTE Datapak::R_port6() {
	return port6.latch;
}

void Datapak::W_port6(BYTE data) {
	port6.latch = data;
	processDataOnPort();
}

void Datapak::W_port6DDR(BYTE data) {
	if (port6.ddr != data) {
		port6.ddr = data;
		processDataOnPort();
	}
}

// Process changes to port2 or port6 and make changes
void Datapak::processDataOnPort() {
	port6.data &= (~port6.ddr); // Ignore all bits set to input
	port6.data |= (port6.latch & port6.ddr); // For bits set to input, use bit in latch

	port6.oe = (port6.data & 0x08) >> 3;
	port6.pgm = (port6.data & 0x04) >> 2;
	port6.smr = (port6.data & 0x02) >> 1;
	port6.sclk = port6.data & 0x01;

	// Pass data to powered paks to process
	if (port6.data & 0x80)		// Slots are powered down
		return;
	if (port6.data & 0x40)		// Slot D is powered
		processDataOnPak(2);
	if (port6.data & 0x20)		// Slot C is powered
		processDataOnPak(1);
	if (port6.data & 0x10)		// Slot B is powered
		processDataOnPak(0);
}

// Process port data pushed to powered paks
void Datapak::processDataOnPak(int id) {
	// SCLK Changed
	if (pak[id].line.sclk != port6.sclk) {
		pak[id].line.sclk = port6.sclk;
		pak[id].addr.counter++;
		pak[id].addr.counter &= pak[id].addr.counter_mask;
	}

	// Counter Reset - if SMR is high
	if (port6.smr != 0) {
		pak[id].addr.counter = 0;
		pak[id].addr.page = 0;
	}

	// Page Increment - if PGM goes high
	if (pak[id].line.pgm != port6.pgm) {
		pak[id].line.pgm = port6.pgm;
		if (port6.pgm != 0) {
			pak[id].addr.page++;
			pak[id].addr.page &= pak[id].addr.page_mask;
		}
	}

	// Write to Segment Counter - SMR is high, OE is high, PGM is low and voltage is 5v
	if (port6.smr && port6.oe && !port6.pgm && (svpp == 5)) {
		pak[id].addr.segment = (port2.latch & pak[id].addr.segment_mask);
	}

	// Read Hardware ID - SMR is high, OE is low and PGM is high
	if (port6.smr && !port6.oe && port6.pgm) {
		if (!pak[id].properties.inserted) {
			// Nothing inserted, return 0
			port2.latch = 0x00;
		}
		else if (pak[id].properties.rampak) {
			// Rampaks have an ID of 1
			port2.latch = 0x01;
		}
		else {
			// All other situations return 1st byte of data
			port2.latch = pak[id].data[0];
		}
	}

	// Normal read from pak - SMR is low and OE is low
	if (!port6.smr && !port6.oe) {
		if (pak[id].properties.inserted) {
			port2.latch = pakData(id);
		}
		else {
			port2.latch = 0x00;
		}
	}

	// Write to pak - SMR is low, OE is high, ReadOnly is false
	// Voltage (SVPP) must be 5v for rampak and 21v for datapaks
	if (!port6.smr && port6.oe) {
		if (!pak[id].properties.readOnly && pak[id].properties.inserted) {
			if (pak[id].properties.rampak) {
				// Rampak - 5v write
				// Rampaks overwrite existing byte
				if (svpp == 5) {
					pak[id].data[pakAddr(id)] = port2.latch;
				}
			}
			else {
				// Datapak - 21v write
				// Datapaks are write once, so data is ANDed against existing byte
				if (svpp == 21) {
					pak[id].data[pakAddr(id)] &= port2.latch;
				}
			}
		}
	}
}