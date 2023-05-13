#include "startui.hh"
#include "sdlhelper.hh"
#include "romstore.hh"

#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#undef main // ????

namespace GBEmu
{

StartUi::StartUi(SdlHelper& sdlHelper, RomStore& romStore)
    :sdlHelper_(sdlHelper),
    romStore_(romStore)
{
}

StartUi::~StartUi()
{
}

void StartUi::ProcessEvent(const SDL_Event& event)
{
    switch (event.type)
    {
		case SDL_MOUSEBUTTONDOWN:
        {
            const int x = event.button.x;
            const int y = event.button.y;

            for(const std::pair<int, SDL_Rect> pair : romRects_)
            {
                const int romId = pair.first;
                const SDL_Rect& rect = pair.second;

                if (rect.x <= x && x <= rect.x + rect.w &&
                    rect.y <= y && y <= rect.y + rect.h)
                {
                    if (loadHandler_) loadHandler_(romId);
                }
            }

            break;
        }

        case SDL_KEYDOWN:
        {
            printf("key down %u\n", (unsigned int)event.key.keysym.sym);
            break;
        }
    }
}

void StartUi::Render()
{
	SDL_SetRenderDrawColor(sdlHelper_.GetRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);

    sdlHelper_.RenderTextTopLeft(50, 50, "Select ROM:");

    romRects_.clear();

    int nextY = 150;
    for(const RomData* const rom : romStore_.GetRoms())
    {
        const SDL_Rect rect = {
            .x = 100,
            .y = nextY,
            .w = 400,
            .h = 80,
        };

        nextY += rect.h + 50;

        romRects_.insert(std::pair<int, SDL_Rect>(rom->id_, rect));

        rectangleRGBA(sdlHelper_.GetRenderer(),
            rect.x, rect.y, rect.x + rect.w, rect.y + rect.h,
            255, 255, 255, 255);

        sdlHelper_.RenderTextCenter(
            rect.x + rect.w / 2,
            rect.y + rect.h / 2,
            rom->name_.c_str());
    }

}

}
