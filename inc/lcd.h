#ifndef LCD_H
#define LCD_H

#include "renderer.h"
#include "global.h"

#define LCD_2LINE       0x00
#define LCD_4LINE       0x01

class Lcd
{
private:
    bool power;
    int mode;
    int nextUpdate;
    int errorState;
    Drawing_Rect virtPos[2][128];

    struct {
        BYTE data[196];
        BYTE addr;
        signed char direction;
    } ram;

    struct {
        BYTE type;
        bool underline;
        bool block;
    } cursor;

    void render();

public:
    Lcd();
	void init();

    void update(bool progressTime);
    void setMode(int mode);
    void setPower(bool powered);
    void hardwareError(int err);

    BYTE R_control();
    void W_control(BYTE data);
    BYTE R_data();
    void W_data(BYTE data);
};

extern Lcd *lcd;

#endif // LCD_H
