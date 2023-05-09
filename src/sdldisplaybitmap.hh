#pragma once

#include <memory>

#include <SDL_rect.h>

#include "emulator/display.hh"

struct SDL_Renderer;
struct SDL_Texture;

namespace GBEmu
{

class SdlDisplayBitmap : public Emulator::DisplayBitmap
{
public:
	SdlDisplayBitmap(SDL_Renderer *renderer, const SDL_Rect& presentRect, int width, int height);

	virtual void Clear() override;
	virtual void DrawPixel(uint8_t x, uint8_t y, uint8_t color) override;
	virtual void Present() override;

private:
	SDL_Renderer * const renderer_;
	const SDL_Rect presentRect_;
	const int width_;
	const int height_;

	std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)> texture_;

	uint8_t *pixels_;
	uint16_t pitch_;
};

}