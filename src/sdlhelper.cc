#include "sdlhelper.hh"
#include "sdldisplaybitmap.hh"
#include "sdlsound.hh"
#include "emulator/display.hh"

#include <string>
#include <cassert>

#include <SDL.h>
#include <SDL_ttf.h>
#undef main // ????

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace GBEmu
{

SdlHelper::SdlHelper()
	:fontFileName_("assets/DejaVuSans.ttf"),
    displayZoomFactor_(4),
	window_(nullptr),
	windowRect_({}),
	renderer_(nullptr),
	font_(nullptr)
{
}

SdlHelper::~SdlHelper()
{
}

int SdlHelper::Initialize()
{
    // Initialize SDL.
	int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	assert(!ret);

#if SDL_VERSION_ATLEAST(2, 24, 0)
	// Determine which display to use based on the mouse position.
	SDL_Point globalMousePos = {};
	SDL_GetGlobalMouseState(&globalMousePos.x, &globalMousePos.y);
	int displayIndexContainingMouse = SDL_GetPointDisplayIndex(&globalMousePos);
	int displayIndexForWindow = displayIndexContainingMouse >= 0
		? displayIndexContainingMouse
	 	: 0;
#else
	int displayIndexForWindow = 0;
#endif

	SDL_Rect displayBounds = {};
	SDL_GetDisplayBounds(displayIndexForWindow, &displayBounds);

	// Determine window size / position.
	int gameboyDisplayWidth, gameboyDisplayHeight;
	GBEmu::Emulator::Display::GetSize(&gameboyDisplayWidth, &gameboyDisplayHeight);

	const int windowWidth = gameboyDisplayWidth * displayZoomFactor_;
	const int windowHeight = gameboyDisplayHeight * displayZoomFactor_ * 2; // TODO 16:9 / 9:16 ?

	const int windowX = displayBounds.x + displayBounds.w / 2 - windowWidth / 2;
	const int windowY = displayBounds.y + displayBounds.h / 2 - windowHeight / 2;

	// Create window & renderer.
	window_ = SDL_CreateWindow("GBEmu", windowX, windowY, windowWidth, windowHeight, 0);
	if (!window_) {
		printf("SDL_CreateWindow() failed: %s\n", SDL_GetError()); 
		return -1;
	}

	windowRect_ = {
		.x = 0,
		.y = 0,
		.w = windowWidth,
		.h = windowHeight,
	};

	renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!renderer_) {
		printf("SDL_CreateRenderer() failed: %s\n", SDL_GetError());
		return -1;
	}

	// Initialize TTF.
    if (TTF_Init()) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        return -1;
    }

    font_ = TTF_OpenFont(fontFileName_.c_str(), 24);
    if(!font_) {
        printf("TTF_OpenFont(%s) failed: %s\n", fontFileName_.c_str(), TTF_GetError());
        return -1;
    }

	// Create emulator components.
	const SDL_Rect displayPresentRect = {
		.x = 0,
		.y = 0,
		.w = gameboyDisplayWidth * displayZoomFactor_,
		.h = gameboyDisplayHeight * displayZoomFactor_
	};

	sdlDisplayBitmap_ = std::make_unique<GBEmu::SdlDisplayBitmap>(renderer_, displayPresentRect);
	sdlSound_ = std::make_unique<GBEmu::SdlSound>();

    return 0;
}

int SdlHelper::Shutdown()
{
    sdlDisplayBitmap_ = NULL; // TODO check
    sdlSound_ = NULL;

	if (renderer_)
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = NULL;
    }
	if (window_)
    {
        SDL_DestroyWindow(window_);
        window_ = NULL;
    }

	SDL_Quit();

    return 0;
}

void SdlHelper::Clear()
{
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0xFF);
    SDL_RenderClear(renderer_);
}

void SdlHelper::Present()
{
    SDL_RenderPresent(renderer_);
}

void SdlHelper::RenderTextTopLeft(int x, int y, const char *text)
{
	SDL_Surface *surface = TTF_RenderText_Solid(font_, text, { 255, 255, 255, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer_, surface);

    const SDL_Rect rect = {
        .x = x,
        .y = y,
        .w = surface->w,
        .h = surface->h,
    };

    SDL_RenderCopy(renderer_, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void SdlHelper::RenderTextCenter(int x, int y, const char *text)
{
    SDL_Surface *surface = TTF_RenderText_Solid(font_, text, { 255, 255, 255, 255});
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer_, surface);

    const SDL_Rect rect = {
        .x = x - surface->w / 2,
        .y = y - surface->h / 2,
        .w = surface->w,
        .h = surface->h,
    };

    SDL_RenderCopy(renderer_, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}


}
