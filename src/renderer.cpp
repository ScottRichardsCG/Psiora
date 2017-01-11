#include "renderer.h"
#include <QFile>
#include "SDL.h" // SDL2

Renderer *renderer;

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
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		setError(RENDERER_ERR_SDLINIT);
		return;
	}
	if (ptrWindow == NULL) {
		fprintf(stderr, "SDL_ No valid window pointer. Making our own window\n");
		setError(RENDERER_ERR_DISPLAY);
		SDL_CreateWindowAndRenderer(504, 170, SDL_WINDOW_SHOWN, &window, &render);
		return;
	}
	else {
		window = SDL_CreateWindowFrom(ptrWindow);
		render = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(render, 504, 170);

	SDL_Surface *splashSurf = resourceToSurface(":gfx/Splash");
    SDL_Surface *fontxp = resourceToSurface(":gfx/HD44780_Font");
    SDL_Surface *fontlz = resourceToSurface(":gfx/HD66780_Font");
    SDL_Surface *fontudg = resourceToSurface(":gfx/UDG_Font");

    if ((splashSurf == NULL) || (fontxp == NULL) || (fontlz == NULL) || (fontudg == NULL))
    {
        if (splashSurf != NULL) SDL_FreeSurface(splashSurf);
        if (fontxp != NULL) SDL_FreeSurface(fontxp);
        if (fontlz != NULL) SDL_FreeSurface(fontlz);
        if (fontudg != NULL) SDL_FreeSurface(fontudg);
        setError(RENDERER_ERR_INTIMGMISSING);
        return;
    }

	splash = SDL_CreateTextureFromSurface(render, splashSurf);
	SDL_FreeSurface(splashSurf);

    SDL_Rect rectfont = { 0, 0, 20, 32 };
    SDL_Rect rectudg = { 0, 0, 20, 4 };

    // Split out ascii and udg characters into video surfaces
	SDL_Surface *tmp;
    int i;
    for (i = 0; i < 256; i++) {
        if (i < 32) {
            udgFont[i] = SDL_CreateRGBSurface(0, 20, 4, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
            rectudg.y = i * 4;
            SDL_BlitSurface(fontudg, &rectudg, udgFont[i], NULL);

			font[0][i] = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 20, 32);
			font[1][i] = SDL_CreateTexture(render, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 20, 32);
        }

		if (i < 8) {
			udgSurf[i] = SDL_CreateRGBSurface(0, 20, 32, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
		}

        if (i >= 32) {
			rectfont.x = (i - 32) * 20;

			tmp = SDL_CreateRGBSurface(0, 20, 32, 32, 0, 0, 0, 0);
			SDL_BlitSurface(fontxp, &rectfont, tmp, NULL);
			font[0][i] = SDL_CreateTextureFromSurface(render, tmp);
			SDL_FreeSurface(tmp);

			tmp = SDL_CreateRGBSurface(0, 20, 32, 32, 0, 0, 0, 0);
			SDL_BlitSurface(fontlz, &rectfont, tmp, NULL);
			font[1][i] = SDL_CreateTextureFromSurface(render, tmp);
			SDL_FreeSurface(tmp);

        }
    }

    // Free all SDL surfaces holding decoded bmp data we no longer need
    SDL_FreeSurface(fontxp);
    SDL_FreeSurface(fontlz);
    SDL_FreeSurface(fontudg);

    backgroundColour(0, 0, 255);
    showSplash();
    update();

    setError(RENDERER_ERR_OKAY);
    return;
}

Renderer::~Renderer()
{
    int i = 0;
    for (i = 0; i < 256; i++)
    {
        if (i < 32) {
			if (udgFont[i])
				SDL_FreeSurface(udgFont[i]);
        }

		if (i < 8) {
			if (udgSurf[i])
				SDL_FreeSurface(udgSurf[i]);
		}

		if (font[0][i] != NULL)
			SDL_DestroyTexture(font[0][i]);
		if (font[1][i] != NULL)
			SDL_DestroyTexture(font[1][i]);
    }

    if (splash != NULL)
        SDL_DestroyTexture(splash);

	SDL_DestroyRenderer(render);
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
	SDL_SetRenderDrawColor(render, red, green, blue, 255);
	SDL_RenderClear(render);
}

void Renderer::update() {
	SDL_RenderPresent(render);
}

void Renderer::showSplash() {
	SDL_RenderCopy(render, splash, NULL, NULL);
}

void Renderer::drawRect(Drawing_Rect *rect, int red, int green, int blue) {
    SDL_Rect sdlrect = { rect->x, rect->y, rect->w, rect->h };
	SDL_SetRenderDrawColor(render, red, green, blue, 255);
	SDL_RenderFillRect(render, &sdlrect);
}

void Renderer::drawFont(int mode, int ascii, Drawing_Rect *rect) {
    SDL_Rect sdlrect = { rect->x, rect->y, rect->w, rect->h };
	SDL_RenderCopy(render, font[mode][ascii], NULL, &sdlrect);
}

void Renderer::updateUdg(int udg, Sint16 posY, int code) {
    SDL_Rect sdlrect = { 0, (Sint16)(posY * 4), 32, 4 };
    SDL_BlitSurface(udgFont[code], NULL, udgSurf[udg], &sdlrect);
	SDL_UpdateTexture(font[0][udg], NULL, udgSurf[udg]->pixels, udgSurf[udg]->pitch);
	SDL_UpdateTexture(font[0][udg + 8], NULL, udgSurf[udg]->pixels, udgSurf[udg]->pitch);
	SDL_UpdateTexture(font[1][udg], NULL, udgSurf[udg]->pixels, udgSurf[udg]->pitch);
	SDL_UpdateTexture(font[1][udg + 8], NULL, udgSurf[udg]->pixels, udgSurf[udg]->pitch);
}
