#ifndef KEYPAD_H
#define KEYPAD_H

#include "global.h"

#define KEYSTATE_F1         0xE001
#define KEYSTATE_F2         0xE002
#define KEYSTATE_UP         0xE003
#define KEYSTATE_DOWN       0xE004
#define KEYSTATE_LEFT       0xE005
#define KEYSTATE_RIGHT      0xE006
#define KEYSTATE_RETURN     0xE007
#define KEYSTATE_SHIFT      0xE008
#define KEYSTATE_BACKSPACE  0xE009
#define KEYSTATE_DELETE     0xE00A

#define KEY_ON				0x00
#define KEY_A				0x01
#define KEY_B				0x02
#define KEY_C				0x03
#define KEY_D				0x04
#define KEY_E				0x05
#define KEY_F				0x06
#define KEY_G				0x07
#define KEY_H				0x08
#define KEY_I				0x09
#define KEY_J				0x0A
#define KEY_K				0x0B
#define KEY_L				0x0C
#define KEY_M				0x0D
#define KEY_N				0x0E
#define KEY_O				0x0F
#define KEY_P				0x10
#define KEY_Q				0x11
#define KEY_R				0x12
#define KEY_S				0x13
#define KEY_T				0x14
#define KEY_U				0x15
#define KEY_V				0x16
#define KEY_W				0x17
#define KEY_X				0x18
#define KEY_Y				0x19
#define KEY_Z				0x1A
#define KEY_SHIFT			0x1B
#define KEY_DEL				0x1C
#define KEY_SPACE			0x1D
#define KEY_EXE				0x1E
#define KEY_MODE			0x1F
#define KEY_UP				0x20
#define KEY_LEFT			0x21
#define KEY_RIGHT			0x22
#define KEY_DOWN			0x23

typedef struct {
    UINT8 k;
    UINT8 p5;
} keyboard_keys;

class Keypad
{
private:
    char state[65536];
    bool ac;
    bool caps;
    char line[8];
public:
    Keypad();
    void keyDown(int key);
    void keyUp();
    bool isOnClearPressed();
    BYTE activeKeyLines(BYTE stage2);
};

extern Keypad *keypad;

#endif // KEYPAD_H
