#ifndef RENDERER_H
#define RENDERER_H

#include "SDL/SDL.h"
#include <QString>

#define RENDERER_ERR_OKAY           0x00
#define RENDERER_ERR_SDLINIT        0x01
#define RENDERER_ERR_DISPLAY        0x02
#define RENDERER_ERR_INTIMGMISSING  0x03

typedef struct {
    Sint16 x;
    Sint16 y;
    Uint16 w;
    Uint16 h;
} Drawing_Rect;

SDL_Surface* resourceToSurface(QString filePath);
int keyboardFilter(const SDL_Event *event);

class Renderer
{
private:
    void* windowId;
    SDL_Surface *canvas;
    SDL_Surface *splash;
    SDL_Surface *font[2][256];
    SDL_Surface *udgFont[32];
    int errorCode;

    void setError(int code);
public:
    Renderer(void* ptrWindow);
    ~Renderer();

    int getError();
    void backgroundColour(int red, int green, int blue);
    void update();
    void showSplash();
    void drawRect(Drawing_Rect *rect, int red, int green, int blue);
    void drawFont(int mode, int ascii, Drawing_Rect *rect);
    void updateUdg(int udg, Sint16 posY, int code);
};

extern Renderer *renderer;

#endif // RENDERER_H
