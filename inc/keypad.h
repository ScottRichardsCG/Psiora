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

#define KEY_A 1
#define KEY_B 2
#define KEY_C 3
#define KEY_D 4
#define KEY_E 5
#define KEY_F 6
#define KEY_G 7
#define KEY_H 8
#define KEY_I 9
#define KEY_J 10
#define KEY_K 11
#define KEY_L 12
#define KEY_M 13
#define KEY_N 14
#define KEY_O 15
#define KEY_P 16
#define KEY_Q 17
#define KEY_R 18
#define KEY_S 19
#define KEY_T 20
#define KEY_U 21
#define KEY_V 22
#define KEY_W 23
#define KEY_X 24
#define KEY_Y 25
#define KEY_Z 26
#define KEY_SHIFT 27
#define KEY_DEL 28
#define KEY_SPACE 29
#define KEY_EXE 30
#define KEY_MODE 31
#define KEY_UP 32
#define KEY_LEFT 33
#define KEY_RIGHT 34
#define KEY_DOWN 35

typedef struct {
    BYTE k;
    BYTE p5;
} keyboard_keys;

class Keypad
{
private:
    char state[65536];
    bool ac;
    bool caps;
    BYTE line[8];
public:
    Keypad();
    void keyDown(int key);
    void keyUp(int key);
    bool isOnClearPressed();
    BYTE activeKeyLines(unsigned int stage2);
};

extern Keypad *keypad;

#endif // KEYPAD_H
