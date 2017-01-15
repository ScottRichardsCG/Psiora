#include "keypad.h"
#include "global.h"
#include "emucore.h"
#include "memory.h"
#include <cstring>

Keypad *keypad;

#define keyActive(psioraKey) line[keyPadBits[psioraKey].k] &= (0xff-keyPadBits[psioraKey].p5)
#define LCK(key1, key2) if (state[key1]) { caps = false; keyActive(key2); }
#define UCK(key1, key2) if (state[key1]) { caps = true; keyActive(key2); }
#define NCK(key1, key2) if (state[key1]) { keyActive(key2); }
#define SCK(key1, key2) if (state[key1]) { keyActive(KEY_SHIFT); keyActive(key2); }

static const keyboard_keys keyPadBits[36] = {
	{ 0, 0x00 },
	{ 2, 0x40 },{ 3, 0x40 },{ 4, 0x40 },{ 7, 0x40 },{ 5, 0x40 },{ 6, 0x40 },
	{ 2, 0x20 },{ 3, 0x20 },{ 4, 0x20 },{ 7, 0x20 },{ 5, 0x20 },{ 6, 0x20 },
	{ 2, 0x10 },{ 3, 0x10 },{ 4, 0x10 },{ 7, 0x10 },{ 5, 0x10 },{ 6, 0x10 },
	{ 2, 0x08 },{ 3, 0x08 },{ 4, 0x08 },{ 7, 0x08 },{ 5, 0x08 },{ 6, 0x08 },
	{ 4, 0x04 },{ 7, 0x04 },{ 2, 0x04 },{ 3, 0x04 },{ 5, 0x04 },{ 6, 0x04 },
	{ 1, 0x04 },{ 1, 0x08 },{ 1, 0x20 },{ 1, 0x40 },{ 1, 0x10 }
};

Keypad::Keypad()
{
    memset(state, 0, 65536);
    ac = false;
    caps = false;
    memset(line, 0xff, 8);
}

void Keypad::keyDown(int key)
{
    if (emucore->isPowered() == true)
    {
        if (key >= 0 && key <= 65536)
        {
            state[key] = 1;
        }
    }
    else {
        if (key == KEYSTATE_F1)
        {
            emucore->setPower(true);
        }
    }
}

void Keypad::keyUp(int key)
{
    memset(state, 0x00, 65536);
}

bool Keypad::isOnClearPressed()
{
    if (state[KEYSTATE_F1])
    {
        return true;
    }
    return false;
}

BYTE Keypad::activeKeyLines(unsigned int stage2)
{
	memset(line, 0xff, 8);

    // FORCE KEYSTAT (Caps Lock, Num Lock off)
	BYTE tmpLck = memory->readDirect(0x007B);	// Read Caps Lock value
    tmpLck &= 0xbe;
    if (!caps)
        tmpLck++;
	memory->writeDirect(0x007B, tmpLck); // Write Caps Lock value

    // IMPORTANT
    // DO NOT CAPTURE SHIFT KEYS

    NCK(KEYSTATE_F2, KEY_MODE);
    NCK(KEYSTATE_UP, KEY_UP);
    NCK(KEYSTATE_DOWN, KEY_DOWN);
    NCK(KEYSTATE_LEFT, KEY_LEFT);
    NCK(KEYSTATE_RIGHT, KEY_RIGHT);
    SCK(KEYSTATE_DELETE, KEY_DEL);
    NCK(KEYSTATE_BACKSPACE, KEY_DEL);
    NCK(KEYSTATE_RETURN, KEY_EXE);
    NCK(' ', KEY_SPACE);

    LCK('a', KEY_A);    UCK('A', KEY_A);    SCK('<', KEY_A);
    LCK('b', KEY_B);    UCK('B', KEY_B);    SCK('>', KEY_B);
    LCK('c', KEY_C);    UCK('C', KEY_C);    SCK('(', KEY_C);
    LCK('d', KEY_D);    UCK('D', KEY_D);    SCK(')', KEY_D);
    LCK('e', KEY_E);    UCK('E', KEY_E);    SCK('%', KEY_E);
    LCK('f', KEY_F);    UCK('F', KEY_F);    SCK('/', KEY_F);
    LCK('g', KEY_G);    UCK('G', KEY_G);    SCK('=', KEY_G);
    LCK('h', KEY_H);    UCK('H', KEY_H);    SCK('"', KEY_H);
    LCK('i', KEY_I);    UCK('I', KEY_I);    SCK('7', KEY_I);
    LCK('j', KEY_J);    UCK('J', KEY_J);    SCK('8', KEY_J);
    LCK('k', KEY_K);    UCK('K', KEY_K);    SCK('9', KEY_K);
    LCK('l', KEY_L);    UCK('L', KEY_L);    SCK('*', KEY_L);
    LCK('m', KEY_M);    UCK('M', KEY_M);    SCK(',', KEY_M);
    LCK('n', KEY_N);    UCK('N', KEY_N);    SCK('$', KEY_N);
    LCK('o', KEY_O);    UCK('O', KEY_O);    SCK('4', KEY_O);
    LCK('p', KEY_P);    UCK('P', KEY_P);    SCK('5', KEY_P);
    LCK('q', KEY_Q);    UCK('Q', KEY_Q);    SCK('6', KEY_Q);
    LCK('r', KEY_R);    UCK('R', KEY_R);    SCK('-', KEY_R);
    LCK('s', KEY_S);    UCK('S', KEY_S);    SCK(';', KEY_S);
    LCK('t', KEY_T);    UCK('T', KEY_T);    SCK(':', KEY_T);
    LCK('u', KEY_U);    UCK('U', KEY_U);    SCK('1', KEY_U);
    LCK('v', KEY_V);    UCK('V', KEY_V);    SCK('2', KEY_V);
    LCK('w', KEY_W);    UCK('W', KEY_W);    SCK('3', KEY_W);
    LCK('x', KEY_X);    UCK('X', KEY_X);    SCK('+', KEY_X);
    LCK('y', KEY_Y);    UCK('Y', KEY_Y);    SCK('0', KEY_Y);
    LCK('z', KEY_Z);    UCK('Z', KEY_Z);    SCK('.', KEY_Z);

    BYTE ret = 0x7c;
    if (!(stage2 & 0x01)) { ret &= line[1]; }
    if (!(stage2 & 0x02)) { ret &= line[2]; }
    if (!(stage2 & 0x04)) { ret &= line[3]; }
    if (!(stage2 & 0x08)) { ret &= line[4]; }
    if (!(stage2 & 0x10)) { ret &= line[5]; }
    if (!(stage2 & 0x20)) { ret &= line[6]; }
    if (!(stage2 & 0x40)) { ret &= line[7]; }
    if (state[KEYSTATE_F1]) { ret |= 0x80; }

	return ret;
}
