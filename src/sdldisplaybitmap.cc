#include "sdldisplaybitmap.hh"

#include <cassert>

#include <SDL.h>
#undef main

namespace GBEmu
{

SdlDisplayBitmap::SdlDisplayBitmap(SDL_Renderer *renderer, int width, int height)
	:renderer_(renderer),
	width_(width),
	height_(height),
	pixels_(nullptr),
	pitch_(0)
{
	assert(renderer);
	assert(width > 0);
	assert(height > 0);

	texture_ = SDL_CreateTexture(renderer_,
		SDL_PIXELFORMAT_BGRA8888,
		SDL_TEXTUREACCESS_STREAMING,
		width,
		height);
}

SdlDisplayBitmap::~SdlDisplayBitmap()
{
	if (texture_)
	{
		SDL_DestroyTexture(texture_);
		texture_ = nullptr;
	}
}

void SdlDisplayBitmap::Clear()
{
	void *pixels = nullptr;
	int pitch = 0;

	SDL_LockTexture(texture_, nullptr, &pixels, &pitch);
	assert(pixels);
	assert(pitch);

	pixels_ = reinterpret_cast<uint8_t*>(pixels);
	pitch_ = uint16_t(pitch);

	memset(pixels_, 0xFF, 4 * width_ * height_);
}

void SdlDisplayBitmap::DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
	if (x >= width_) return;
	if (y >= height_) return;
	if (!pixels_) return;

	uint32_t offset = y * pitch_ + x * 4;

	pixels_[offset + 0] = 255;
	pixels_[offset + 1] = color;
	pixels_[offset + 2] = color;
	pixels_[offset + 3] = color;

}

void SdlDisplayBitmap::Present()
{
	if (!pixels_) return;

	SDL_UnlockTexture(texture_);
	SDL_RenderCopy(renderer_, texture_, NULL, NULL);
	SDL_RenderPresent(renderer_);

	pixels_ = nullptr;
	pitch_ = 0;
}

}
