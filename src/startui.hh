#pragma once

#include <array>
#include <string>
#include <cstdbool>
#include <functional>
#include <map>

#include <SDL_rect.h>
#include <SDL_touch.h>

struct SDL_Renderer;
typedef union SDL_Event SDL_Event;
typedef struct _TTF_Font TTF_Font;

namespace GBEmu
{

class SdlHelper;
class RomStore;

class StartUi
{
public:
    StartUi(SdlHelper& sdlHelper, RomStore& romStore);
    virtual ~StartUi();

    void ProcessEvent(const SDL_Event& event);
    void Render();

    void RegisterLoad(std::function<void(int)> handler) {
        loadHandler_ = handler;
    }

private:
    SdlHelper& sdlHelper_;
    RomStore& romStore_;

    std::function<void(int)> loadHandler_;

    std::map<int, SDL_Rect> romRects_;
};

}