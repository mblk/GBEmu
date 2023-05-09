#include "touchui.hh"

#include "emulator/keypad.hh"

#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>
#undef main

namespace GBEmu
{

static std::array<TouchPoint, 8> GetTouchPoints(const SDL_Rect& displayRect, const SDL_Rect& uiRect)
{
    const int xUnit = uiRect.w / 8.0f;
    const int yUnit = uiRect.h / 8.0f;

    std::array<TouchPoint, 8> result
    {{
        // left
        {
            .position_ = {
                .x = (int)(uiRect.x + xUnit * 1.5f),
                .y = (int)(uiRect.y + yUnit * 2.5f),
            },
            .index_ = GBEmu::Emulator::Keypad::Keys::Left,
            .label_ = "Left",
        },
        // right
        {
            .position_ = {
                .x = (int)(uiRect.x + xUnit * 3.5f),
                .y = (int)(uiRect.y + yUnit * 2.5f),
            },
            .index_ = GBEmu::Emulator::Keypad::Keys::Right,
            .label_ = "Right",
        },
        // up
        {
            .position_ = {
                .x = (int)(uiRect.x + xUnit * 2.5f),
                .y = (int)(uiRect.y + yUnit * 1.5f),
            },
            .index_ = GBEmu::Emulator::Keypad::Keys::Up,
            .label_ = "Up",
        },
        // down
        {
            .position_ = {
                .x = (int)(uiRect.x + xUnit * 2.5f),
                .y = (int)(uiRect.y + yUnit * 3.5f),
            },
            .index_ = GBEmu::Emulator::Keypad::Keys::Down,
            .label_ = "Down",
        },
        // a
        {
            .position_ = {
                .x = (int)(uiRect.x + xUnit * 6.5f),
                .y = (int)(uiRect.y + yUnit * 2.0f),
            },
            .index_ = GBEmu::Emulator::Keypad::Keys::A,
            .label_ = "A",
        },
        // b
        {
            .position_ = {
                .x = (int)(uiRect.x + xUnit * 5.5f),
                .y = (int)(uiRect.y + yUnit * 3.0f),
            },
            .index_ = GBEmu::Emulator::Keypad::Keys::B,
            .label_ = "B",
        },
        // select
        {
            .position_ = {
                .x = (int)(uiRect.x + xUnit * 3.0f),
                .y = (int)(uiRect.y + yUnit * 6.0f),
            },
            .index_ = GBEmu::Emulator::Keypad::Keys::Select,
            .label_ = "Select",
        },
        // start
        {
            .position_ = {
                .x = (int)(uiRect.x + xUnit * 5.0f),
                .y = (int)(uiRect.y + yUnit * 6.0f),
            },
            .index_ = GBEmu::Emulator::Keypad::Keys::Start,
            .label_ = "Start",
        }
    }};

    return result;
}

TouchUi::TouchUi(SDL_Renderer *renderer, const SDL_Rect& displayRect, const SDL_Rect& uiRect)
    :renderer_(renderer),
    displayRect_(displayRect),
    uiRect_(uiRect),
    touchPoints_(GetTouchPoints(displayRect, uiRect)),
    contacts_({})
{
}

int TouchUi::Initialize(const std::string &fontFileName)
{
    if (TTF_Init()) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        return -1;
    }

    font_ = TTF_OpenFont(fontFileName.c_str(), 24);
    if(!font_) {
        printf("TTF_OpenFont(%s) failed: %s\n", fontFileName.c_str(), TTF_GetError());
        return -1;
    }

    return 0;
}

void TouchUi::ProcessEvent(const SDL_Event& event)
{
    switch (event.type)
    {
		case SDL_MOUSEBUTTONDOWN:
        {
            const SDL_FingerID finger = (SDL_FingerID)event.button.button;
            const TouchPoint* closestTouchPoint = GetClosest(event.button.x, event.button.y);
            if (closestTouchPoint)
            {
                AddContact(finger, closestTouchPoint->index_);
            }
            break;
        }

		case SDL_MOUSEBUTTONUP:
        {
            const SDL_FingerID finger = (SDL_FingerID)event.button.button;
            RemoveContact(finger);
			break;
        }

		case SDL_FINGERDOWN:
        {
            int x = (int)(event.tfinger.x * (float)displayRect_.w);
            int y = (int)(event.tfinger.y * (float)displayRect_.h);
            const TouchPoint* closestTouchPoint = GetClosest(x, y);
            if (closestTouchPoint)
            {
                AddContact(event.tfinger.fingerId, closestTouchPoint->index_);
            }
			break;
        }

		case SDL_FINGERUP:
        {
            RemoveContact(event.tfinger.fingerId);
			break;
        }
    }
}

void TouchUi::GetKeyStates(std::array<bool, 8> &output) const
{
    for(const Contact& contact : contacts_)
    {
        if (contact.active_)
        {
            output[contact.key_] = true;
        }
    }
}

const TouchPoint* TouchUi::GetClosest(int x, int y) const
{
    const TouchPoint* closest = NULL;
    float closestDistance = 0;

    for(const TouchPoint& touchPoint : touchPoints_)
    {
        int px = touchPoint.position_.x;
        int py = touchPoint.position_.y;

        float distance = sqrt((x-px)*(x-px) + (y-py)*(y-py));

        if (closest == NULL || distance < closestDistance)
        {
            closest = &touchPoint;
            closestDistance = distance;
        }
    }

    return closest;
}

void TouchUi::AddContact(SDL_FingerID finger, int key)
{
    RemoveContact(finger);

    for(Contact& contact : contacts_)
    {
        if (!contact.active_)
        {
            contact.active_ = true;
            contact.finger_ = finger;
            contact.key_ = key;
            return;
        }
    }

    printf("No space to store contact");
}

void TouchUi::RemoveContact(SDL_FingerID finger)
{
    for(Contact& contact : contacts_)
    {
        if (contact.finger_ == finger)
        {
            contact.active_ = false;
            contact.finger_ = 0;
            contact.key_ = 0;
        }
    }
}

void TouchUi::Render()
{
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0xFF);
    SDL_RenderClear(renderer_);

	SDL_SetRenderDrawColor(renderer_, 0xFF, 0xFF, 0xFF, 0xFF);

	std::array<bool, 8> keyStates = {};
	GetKeyStates(keyStates);

    for(const TouchPoint& touchPoint : touchPoints_)
    {
        const SDL_Point& p = touchPoint.position_;

        if (keyStates[touchPoint.index_])
            filledCircleRGBA(renderer_, p.x, p.y, 50, 255, 255, 255, 255);
        else
            circleRGBA(renderer_, p.x, p.y, 50, 255, 255, 255, 255);

        RenderTextSlow(p.x, p.y, touchPoint.label_);
    }
}

void TouchUi::RenderTextSlow(int x, int y, const char *text)
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
