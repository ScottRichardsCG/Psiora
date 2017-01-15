#include "lcd.h"
#include "renderer.h"
#include "global.h"

Lcd *lcd;

static signed int lcdVirtualPosX[2][128] = {
    {
        62, 86, 110, 134, 158, 182, 206, 230, 254, 278, 302, 326, 350, 374, 398, 422,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        62, 86, 110, 134, 158, 182, 206, 230, 254, 278, 302, 326, 350, 374, 398, 422,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        14, 38, 62, 86, 14, 38, 62, 86, 110, 134, 158, 182, 206, 230, 254, 278,
        110, 134, 158, 182, 206, 230, 254, 278, 302, 326, 350, 374, 398, 422, 446, 470,
        302, 326, 350, 374, 398, 422, 446, 470, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        14, 38, 62, 86, 14, 38, 62, 86, 110, 134, 158, 182, 206, 230, 254, 278,
        110, 134, 158, 182, 206, 230, 254, 278, 302, 326, 350, 374, 398, 422, 446, 470,
        302, 326, 350, 374, 398, 422, 446, 470, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

static signed int lcdVirtualPosY[2][128] = {
    {
        50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }, {
        14, 14, 14, 14, 86, 86, 86, 86, 14, 14, 14, 14, 14, 14, 14, 14,
        86, 86, 86, 86, 86, 86, 86, 86, 14, 14, 14, 14, 14, 14, 14, 14,
        86, 86, 86, 86, 86, 86, 86, 86, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        50, 50, 50, 50, 122, 122, 122, 122, 50, 50, 50, 50, 50, 50, 50, 50,
        122, 122, 122, 122, 122, 122, 122, 122, 50, 50, 50, 50, 50, 50, 50, 50,
        122, 122, 122, 122, 122, 122, 122, 122, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};


Lcd::Lcd()
{
    int i = 0;
    for (i = 0; i < 2; i++) {
        int j = 0;
        for (j = 0; j < 128; j++) {
            virtPos[i][j].x = lcdVirtualPosX[i][j];
            virtPos[i][j].y = lcdVirtualPosY[i][j];
            virtPos[i][j].w = 20;
            virtPos[i][j].h = 32;
        }
    }

	mode = 0; // Default to HD44780 - 2 Line mode
	init();
}

void Lcd::init() {
	power = false;
	memset(ram.data, 0, 196);
	ram.addr = 0;
	ram.direction = 1;
	cursor.type = 0;
	cursor.underline = false;
	cursor.block = false;
	nextUpdate = 0;
	errorState = 0;

	renderer->showSplash();
	renderer->update();
}

void Lcd::render() {
    // Draw Background with error code
    // Background Colours:     Blue    -   Okay
    //                         Red     -   CPU Error
    //                         Yellow  -   Illegal OP Code
    switch (errorState)
    {
    case 0: renderer->backgroundColour(0, 0, 255); break;
    case 1: renderer->backgroundColour(255, 0, 0); break;
    case 2: renderer->backgroundColour(255, 255, 0); break;
    }

    // Draw LCD Background
    Drawing_Rect rect = { 10, 10, 484, 148 };
    renderer->drawRect(&rect, 220, 220, 220);

    // Draw LCD Display
    int i;
    for (i = 0; i<128; i++) {
        if (virtPos[mode][i].x != 0)
            renderer->drawFont(mode, ram.data[i], &virtPos[mode][i]);
    }
    update(false);
}

// Called every 50ms (20 times a second). Updates cursor
// state when required and causes rendering of the buffer to
// the screen as to replicate the LCD refresh rate.
void Lcd::update(bool progressTime) {
    // If LCD is off, just display the splash image
    if (power == false) {
        renderer->showSplash();
        renderer->update();
        return;
    }

    // Update cursor state every 500ms
    if (progressTime == true) {
        nextUpdate++;
        if (nextUpdate >= 10) {
            nextUpdate = 0;
            cursor.type = (cursor.type + 1) & 0x01;
        }
    }

    // Draw cursor over current buffer
    if (ram.addr < 0x80) {
        if (virtPos[mode][ram.addr].x != 0) {
            if (cursor.type == 0 && cursor.underline == true) {
                Drawing_Rect rect = { virtPos[mode][ram.addr].x, Sint16(virtPos[mode][ram.addr].y + 27), 20, 4 };
                renderer->drawRect(&rect, 0, 0, 0);
            }
            else if (cursor.type == 1 && cursor.block == true) {
                renderer->drawRect(&virtPos[mode][ram.addr], 0, 0, 0);
            }
        }
    }

    // Draw buffer to screen
    renderer->update();

    // Remove Cursor from buffer now we have done the refresh
    // Failure to do this can result in cursor artefacts if the cursor moves,
    // but its old position is not updated before the next render to display
    if (ram.addr < 0x80) {
        if (virtPos[mode][ram.addr].x != 0) {
            renderer->drawFont(mode, ram.data[ram.addr], &virtPos[mode][ram.addr]);
        }
    }
}

// Change LCD background colour to signify an internal error
void Lcd::hardwareError(int err) {
    errorState = err;
    render();
}

// Read/Write of the Control Line
BYTE Lcd::R_control() {
    // Read current LCD state and address. However in emulation the LCD
    // is never busy or overloaded. So only return the current address.
    return (ram.addr & 0x7f);
}

void Lcd::W_control(BYTE data) {
    if (power == false)
        return;

    if (data == 1) {
        // Clear screen, and set cursor to home position
        memset(ram.data, 32, 128);
        ram.addr = 0;
        render();
    }
    else if (data < 4) {
        // Set cursor to home positon
        ram.addr = 0;
    }
    else if (data < 8) {
        // Change memory accessing direction
        if (data & 2)
            ram.direction = 1;
        else
            ram.direction = -1;
    }
    else if (data < 16) {
        // Change cursor states.
        if (data & 2)
            cursor.underline = true;
        else
            cursor.underline = false;
        if (data & 1)
            cursor.block = true;
        else
            cursor.block = false;
    }
    else if (data < 32) {
        // Move the current memory address location
        if ((data & 8) == 0) {
            if (data & 4)
                ram.addr--;
            else
                ram.addr++;
        }
    }
    else if (data < 64) {
        // Do nothing for now
        // Data Port, 2 Line and Raster Mode are not emulated.
        // Not really needed as not used in production.
    }
    else if (data < 128) {
        // Set to udg editing mode and move memory address to the udg area
        ram.addr = 0x80 + (data & 0x3F);
    }
    else {
        // Set to lcd editing mode and move memory address to the screen area
        ram.addr = data & 0x7F;
    }
}

// Read/Write Data Line

BYTE Lcd::R_data() {
    // Read data at current memory address, then increment or
    // decrement address depending on current mode
    unsigned char ret = ram.data[ram.addr];
    ram.addr += ram.direction;
    return ret;
}

void Lcd::W_data(BYTE data) {
    if (power == false)
        return;

    if (ram.addr & 0x80) {
        // Update UDG data and update UDG texture in video memory.
        ram.data[ram.addr] = data & 0x1f;
        renderer->updateUdg(((ram.addr & 0x7f) >> 3), (ram.addr & 7), (data & 0x1f));
        render();
    }
    else {
        // Update LCD data and draw new character to screen if visible.
        ram.data[ram.addr] = data;
        if (&virtPos[mode][ram.addr].x != 0)
            renderer->drawFont(mode, data, &virtPos[mode][ram.addr]);
    }
    // Increment or decrement memory address
    ram.addr += ram.direction;
    update(false);
}

// Set current screen emulation mode
// 0 = HD44780 - 2 Line Mode
// 1 = HD66780 - 4 Line Mode
void Lcd::setMode(int val) {
    mode = val;
    render();
}

// Switch display on or off. Switching off also erases all
// data currently held in LCD memory and resets address to 0
void Lcd::setPower(bool powered) {
    if (powered == true) {
        power = true;
        memset(ram.data, 0, 196);
        memset(ram.data, 32, 128);
        ram.addr = 0;
        ram.direction = 1;
        cursor.type = 0;
        cursor.underline = false;
        cursor.block = false;
    }
    else {
        power = false;
    }
    render();
}
