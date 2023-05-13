#pragma once

#include <array>
#include <string>
#include <cstdbool>
#include <functional>

#include <SDL_rect.h>
#include <SDL_touch.h>

struct SDL_Renderer;
typedef union SDL_Event SDL_Event;
typedef struct _TTF_Font TTF_Font;

namespace GBEmu
{

class SdlHelper;

struct TouchPoint
{
    const SDL_Point position_;
    const int index_;
    const char * const label_;
};

struct Contact
{
    bool active_;
    int key_;
    SDL_FingerID finger_;
};

class TouchUi
{
public:
    TouchUi(SdlHelper& sdlHelper, const SDL_Rect& displayRect, const SDL_Rect& uiRect);

    void ProcessEvent(const SDL_Event& event);
    void Render();
    void GetKeyStates(std::array<bool, 8> &output) const;

    void RegisterExit(std::function<void()> handler) {
        exitHandler_ = handler;
    }

private:
    const TouchPoint* GetClosest(int x, int y) const;
    void AddContact(SDL_FingerID finger, int key);
    void RemoveContact(SDL_FingerID finger);

private:
    SdlHelper& sdlHelper_;
    const SDL_Rect displayRect_;
    const SDL_Rect uiRect_;
    const bool useTouchInput_;

    const std::array<TouchPoint, 8> touchPoints_;
    std::array<Contact, 8> contacts_;
    std::array<bool, 8> keyboardKeyStates_;

    std::function<void()> exitHandler_;
};

}
