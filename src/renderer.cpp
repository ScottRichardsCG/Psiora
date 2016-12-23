#include "renderer.h"
#include "keypad.h"
#include <QFile>
#include "SDL/SDL.h"

Renderer *renderer;

int keyboardFilter(const SDL_Event *event) {
    int keyCode;
    switch (event->type) {
    case SDL_KEYDOWN:
        switch (event->key.keysym.sym) {
        case SDLK_F1:
            keyCode = KEYSTATE_F1; break;
        case SDLK_F2:
            keyCode = KEYSTATE_F2; break;
        case SDLK_UP:
            keyCode = KEYSTATE_UP; break;
        case SDLK_DOWN:
            keyCode = KEYSTATE_DOWN; break;
        case SDLK_LEFT:
            keyCode = KEYSTATE_LEFT; break;
        case SDLK_RIGHT:
            keyCode = KEYSTATE_RIGHT; break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            keyCode = KEYSTATE_RETURN; break;
        case SDLK_DELETE:
            keyCode = KEYSTATE_DELETE; break;
        case SDLK_BACKSPACE:
            keyCode = KEYSTATE_BACKSPACE; break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT:
            keyCode = KEYSTATE_SHIFT; break;
        default:
            keyCode = event->key.keysym.unicode; break;
        }
        keypad->keyDown(keyCode);
    case SDL_KEYUP:
        keypad->keyUp();
    }
    return 1;
}

SDL_Surface* resourceToSurface(QString filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return NULL;
    }
    QByteArray data = file.readAll();
    file.close();
    SDL_Surface* img = SDL_LoadBMP_RW(SDL_RWFromMem(data.data(), data.size()), 0);
    return img;
}

Renderer::Renderer(void* ptrWindow)
{
    if (ptrWindow != NULL) {
        char winhack[1024];
        snprintf(winhack, 1024, "SDL_WINDOWID=%ld", (long int)ptrWindow);
        SDL_putenv(winhack);
    }
    else {
        fprintf(stderr, "SDL: No valid window pointer. Making our own window\n");
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        setError(RENDERER_ERR_SDLINIT);
        return;
    }

    canvas = SDL_SetVideoMode(504, 170, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (canvas == NULL) {
        setError(RENDERER_ERR_DISPLAY);
        return;
    }

    splash = resourceToSurface(":gfx/Splash");
    SDL_Surface *fontxp = resourceToSurface(":gfx/HD44780_Font");
    SDL_Surface *fontlz = resourceToSurface(":gfx/HD66780_Font");
    SDL_Surface *fontudg = resourceToSurface(":gfx/UDG_Font");

    if ((splash == NULL) || (fontxp == NULL) || (fontlz == NULL) || (fontudg == NULL))
    {
        if (splash != NULL) SDL_FreeSurface(splash);
        if (fontxp != NULL) SDL_FreeSurface(fontxp);
        if (fontlz != NULL) SDL_FreeSurface(fontlz);
        if (fontudg != NULL) SDL_FreeSurface(fontudg);
        setError(RENDERER_ERR_INTIMGMISSING);
        return;
    }

    SDL_Rect rectfont = { 0, 0, 20, 32 };
    SDL_Rect rectudg = { 0, 0, 20, 4 };

    // Split out ascii and udg characters into video surfaces
    int i;
    for (i = 0; i < 256; i++) {
        if (i < 32) {
            udgFont[i] = SDL_CreateRGBSurface(0, 20, 4, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
            rectudg.y = i * 4;
            SDL_BlitSurface(fontudg, &rectudg, udgFont[i], NULL);
        }

        font[0][i] = SDL_CreateRGBSurface(0, 20, 32, 32, 0, 0, 0, 0);
        font[1][i] = SDL_CreateRGBSurface(0, 20, 32, 32, 0, 0, 0, 0);

        if (i >= 32) {
            rectfont.x = (i - 32) * 20;
            SDL_BlitSurface(fontxp, &rectfont, font[0][i], NULL);
            SDL_BlitSurface(fontlz, &rectfont, font[1][i], NULL);
        }
    }

    // Free all SDL surfaces holding decoded bmp data we no longer need
    SDL_FreeSurface(fontxp);
    SDL_FreeSurface(fontlz);
    SDL_FreeSurface(fontudg);

    backgroundColour(0, 0, 255);
    showSplash();
    update();

    SDL_EnableUNICODE(1);
    SDL_SetEventFilter(keyboardFilter);
    setError(RENDERER_ERR_OKAY);
    return;
}

Renderer::~Renderer()
{
    int i = 0;
    for (i = 0; i < 256; i++)
    {
        if (i < 32)
        {
            if (udgFont[i] != NULL)
                SDL_FreeSurface(udgFont[i]);
        }
        if (font[0][i] != NULL)
            SDL_FreeSurface(font[0][i]);
        if (font[1][i] != NULL)
            SDL_FreeSurface(font[1][i]);
    }

    if (splash != NULL)
        SDL_FreeSurface(splash);

    SDL_Quit();
    return;
}

void Renderer::setError(int code)
{
    errorCode = code;
}

int Renderer::getError()
{
    return errorCode;
}

void Renderer::backgroundColour(int red, int green, int blue)
{
    SDL_FillRect(canvas, NULL, SDL_MapRGB(canvas->format, red, green, blue));
}

void Renderer::update() {
    SDL_Flip(canvas);
}

void Renderer::showSplash() {
    SDL_BlitSurface(splash, NULL, canvas, NULL);
}

void Renderer::drawRect(Drawing_Rect *rect, int red, int green, int blue) {
    SDL_Rect sdlrect = { rect->x, rect->y, rect->w, rect->h };
    SDL_FillRect(canvas, &sdlrect, SDL_MapRGB(canvas->format, red, green, blue));
}

void Renderer::drawFont(int mode, int ascii, Drawing_Rect *rect) {
    SDL_Rect sdlrect = { rect->x, rect->y, rect->w, rect->h };
    SDL_BlitSurface(font[mode][ascii], NULL, canvas, &sdlrect);
}

void Renderer::updateUdg(int udg, Sint16 posY, int code) {
    SDL_Rect sdlrect = { 0, (Sint16)(posY * 4), 32, 4 };
    SDL_BlitSurface(udgFont[code], NULL, font[0][udg], &sdlrect);
    SDL_BlitSurface(udgFont[code], NULL, font[1][udg], &sdlrect);
}
