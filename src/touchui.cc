#include "touchui.hh"
#include "sdlhelper.hh"
#include "emulator/keypad.hh"

#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#undef main // ?????

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

TouchUi::TouchUi(SdlHelper& sdlHelper, const SDL_Rect& displayRect, const SDL_Rect& uiRect)
    :sdlHelper_(sdlHelper),
    displayRect_(displayRect),
    uiRect_(uiRect),
    useTouchInput_(SDL_GetNumTouchDevices() > 0),
    touchPoints_(GetTouchPoints(displayRect, uiRect)),
    contacts_({}),
    keyboardKeyStates_({})
{
}

void TouchUi::ProcessEvent(const SDL_Event& event)
{
    // Helper function that translates sdl keys to emulator keys.
	auto TranslateKeyCode = [](SDL_Keycode keyCode) -> int {
		switch (keyCode)
		{
		case SDLK_a: return GBEmu::Emulator::Keypad::Left;
		case SDLK_d: return GBEmu::Emulator::Keypad::Right;
		case SDLK_w: return GBEmu::Emulator::Keypad::Up;
		case SDLK_s: return GBEmu::Emulator::Keypad::Down;
		case SDLK_j: return GBEmu::Emulator::Keypad::A;
		case SDLK_k: return GBEmu::Emulator::Keypad::B;
		case SDLK_e: return GBEmu::Emulator::Keypad::Start;
		case SDLK_q: return GBEmu::Emulator::Keypad::Select;
		default: return -1;
		}
	};

    switch (event.type)
    {
		case SDL_MOUSEBUTTONDOWN:
        {
            if (event.button.y < uiRect_.y)
            {
                if (exitHandler_)
                    exitHandler_();
                return;
            }

            if (!useTouchInput_) {
                const SDL_FingerID finger = (SDL_FingerID)event.button.button;
                const TouchPoint* closestTouchPoint = GetClosest(event.button.x, event.button.y);
                if (closestTouchPoint)
                    AddContact(finger, closestTouchPoint->index_);
            }
            break;
        }

		case SDL_MOUSEBUTTONUP:
        {
            if (!useTouchInput_) {
                const SDL_FingerID finger = (SDL_FingerID)event.button.button;
                RemoveContact(finger);
            }
			break;
        }

		case SDL_FINGERDOWN:
        case SDL_FINGERMOTION:
        {
            if (useTouchInput_) {
                int x = (int)(event.tfinger.x * (float)displayRect_.w);
                int y = (int)(event.tfinger.y * (float)displayRect_.h);
                const TouchPoint* closestTouchPoint = GetClosest(x, y);
                if (closestTouchPoint)
                    AddContact(event.tfinger.fingerId, closestTouchPoint->index_);
            }
			break;
        }

		case SDL_FINGERUP:
        {
            if (useTouchInput_) {
                RemoveContact(event.tfinger.fingerId);
            }
			break;
        }

        case SDL_KEYUP:
        case SDL_KEYDOWN:
        {
            int key = TranslateKeyCode(event.key.keysym.sym);
			if (key >= 0 && key < 8)
			{
				keyboardKeyStates_[key] = event.type == SDL_KEYDOWN;
			}
            break;
        }
    }
}

void TouchUi::GetKeyStates(std::array<bool, 8> &output) const
{
    for(const Contact& contact : contacts_)
    {
        if (contact.active_ && contact.key_ < 8)
        {
            output[contact.key_] = true;
        }
    }

    for(size_t i=0; i<keyboardKeyStates_.size(); i++)
    {
        output[i] |= keyboardKeyStates_[i];
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
    // Update active contact?
    for(Contact& contact : contacts_)
    {
        if (contact.active_ && contact.finger_ == finger)
        {
            contact.key_ = key;
            return;
        }
    }

    // Create new contact for this finger.
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
    SDL_Renderer *renderer = sdlHelper_.GetRenderer();

	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	std::array<bool, 8> keyStates = {};
	GetKeyStates(keyStates);

    for(const TouchPoint& touchPoint : touchPoints_)
    {
        const SDL_Point& p = touchPoint.position_;

        if (keyStates[touchPoint.index_])
            filledCircleRGBA(renderer, p.x, p.y, 50, 255, 255, 255, 255);
        else
            circleRGBA(renderer, p.x, p.y, 50, 255, 255, 255, 255);

        sdlHelper_.RenderTextCenter(p.x, p.y, touchPoint.label_);
    }
}

}
