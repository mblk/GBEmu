#include "sdldisplaybitmap.hh"

#include <cassert>

#include <SDL.h>
#undef main

namespace GBEmu
{

SdlDisplayBitmap::SdlDisplayBitmap(SDL_Renderer *renderer, const SDL_Rect& presentRect)
	:renderer_(renderer),
	presentRect_(presentRect),
	width_(GBEmu::Emulator::Display::GetWidth()),
	height_(GBEmu::Emulator::Display::GetHeight()),
	textures_{ {nullptr, nullptr}, {nullptr, nullptr} },
	activeTextureIndex_(0),
	pixels_(nullptr),
	pitch_(0)
{
	// Assert before trying to initialize the unique_ptr.
	assert(renderer);
	assert(width_ > 0);
	assert(height_ > 0);

	for(int i=0; i<2; i++)
	{
		textures_[i] = std::unique_ptr<SDL_Texture, void(*)(SDL_Texture*)>(
			SDL_CreateTexture(renderer_,
				SDL_PIXELFORMAT_BGRA8888,
				SDL_TEXTUREACCESS_STREAMING,
				width_,
				height_),
			SDL_DestroyTexture);
	}
}

void SdlDisplayBitmap::Clear()
{
	auto& activeTexture = textures_[activeTextureIndex_];

	void *pixels = nullptr;
	int pitch = 0;

	SDL_LockTexture(activeTexture.get(), nullptr, &pixels, &pitch);
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

	auto& activeTexture = textures_[activeTextureIndex_];

	SDL_UnlockTexture(activeTexture.get());
	//SDL_RenderCopy(renderer_, texture_.get(), NULL, &presentRect_);

	pixels_ = nullptr;
	pitch_ = 0;

	activeTextureIndex_ = (activeTextureIndex_ + 1) % 2;
}

void SdlDisplayBitmap::Render()
{
	auto& renderTexture = textures_[(activeTextureIndex_ + 1) % 2];

	SDL_RenderCopy(renderer_, renderTexture.get(), NULL, &presentRect_);
}

}
