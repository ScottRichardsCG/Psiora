#ifndef DATAPAK_H
#define DATAPAK_H

#include <string>

#include "global.h"

#define MAX_PAK_SIZE			0x10000		// 65536 Bytes (64KB)
#define PAK_SUCCESSFUL			0x00
#define PAK_ERR_CANT_OPEN		0x01
#define PAK_ERR_NOT_OPK			0x02
#define PAK_ERR_TOO_LARGE		0x03

typedef struct {
	std::string filename;
	unsigned int siz;
	unsigned char data[MAX_PAK_SIZE];

	// Slot properties
	struct {
		bool rampak;
		bool paged;
		bool inserted;
		bool readOnly;
	} properties;

	// Last known Datapak lines for this slot
	// This is for detecting a pull to High or Low (change from 0-1 or 1-0)
	struct {
		unsigned char pgm;			// Program mode
		unsigned char sclk;			// Lowest bit of address line
	} line;

	// Addressing
	struct {
		unsigned int counter;
		unsigned int counter_mask;
		unsigned int page;
		unsigned int page_mask;
		unsigned int segment;
		unsigned int segment_mask;
	} addr;
} Struct_datapakSlot;

class Datapak
{
private:
	Struct_datapakSlot pak[3];

	struct {
		unsigned char latch;		// Data currently held on port but unread
		unsigned char ddr;			// Data direction
	} port2;

	struct {
		unsigned char latch;		// Data currently held on port but unread
		unsigned char ddr;			// Data direction
		unsigned char data;			// Current data of port buffer
		unsigned char sclk;			// Lowest bit of address line
		unsigned char smr;			// Master Reset
		unsigned char pgm;			// Program mode
		unsigned char oe;			// Output enable
	} port6;

	int svpp;						// Current voltage to datapak (3, 5 or 21)

	void processDataOnPort();
	void processDataOnPak(int id);

public:
    Datapak();

	bool eject(int id, bool force);
	bool create(bool ram, unsigned int siz, std::string filename);
	bool save(int id);
	int load(int id, std::string filename);

	BYTE R_port2();
	void W_port2(BYTE data);
	void W_port2DDR(BYTE data);

	BYTE R_port6();
	void W_port6(BYTE data);
	void W_port6DDR(BYTE data);

	void setVoltage21(bool volt);
};

extern Datapak *datapak;

#endif // DATAPAK_H