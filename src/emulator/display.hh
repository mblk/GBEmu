#pragma once

#include "memory.hh"

#include <cstdint>
#include <cstdint>
#include <array>

namespace GBEmu::Emulator
{

class IO;
class Pic;
class Ram;

class SpriteAttributeTable : public MemoryRegion
{
public:
	SpriteAttributeTable();
	virtual ~SpriteAttributeTable();

	virtual uint16_t GetSize() const override { return size_; }
	virtual uint8_t Read(uint16_t offset) override;
	virtual void Write(uint16_t offset, uint8_t data) override;

private:
	static constexpr size_t size_ = 160;
	std::array<uint8_t, size_> table_;
};

class DisplayBitmap
{
public:
	virtual void Clear() = 0;
	virtual void DrawPixel(uint8_t x, uint8_t y, uint8_t color) = 0;
	virtual void Present() = 0;
};

class Display
{
public:
	Display(IO &io, Pic &pic, Ram &vram, SpriteAttributeTable &oam, DisplayBitmap *debugBitmap, DisplayBitmap &displayBitmap);
	virtual ~Display();

	void Tick(int ticksPassed);

	static void GetSize(int *width, int *height)
	{
		if (width) *width = 160;
		if (height) *height = 144;
	}

private:
	void DrawLine(uint8_t y);
	void DrawLineBackgroundTile(uint8_t index, uint8_t x, uint8_t y, uint8_t lineOffsetY);
	void DrawLineSpriteTile(uint8_t index, uint8_t x, uint8_t y, uint8_t flags, uint8_t lineOffsetY);
	void DrawLineTile(uint16_t tileDataOffset, uint8_t destX, uint8_t destY, uint8_t lineOffsetY, bool ignore0, bool flipX, bool flipY, bool scrollX, bool scrollY);

	void DrawDebug();
	void DrawDebugBackgroundTile(uint8_t index, uint8_t x, uint8_t y);
	void DrawDebugSpriteTile(uint8_t index, uint8_t x, uint8_t y, uint8_t flags);
	void DrawDebugTile(uint16_t tileDataOffset, uint8_t destX, uint8_t destY, bool ignore0, bool flipX, bool flipY);

private:
	IO &io_;
	Pic &pic_;
	Ram &vram_;
	SpriteAttributeTable &oam_;
	DisplayBitmap *debugBitmap_;
	DisplayBitmap &displayBitmap_;

	int lyTicks_;

	uint8_t lcdc_;
	uint8_t lcds_;

	uint8_t scx_;
	uint8_t scy_;
	uint8_t ly_;
	uint8_t lyc_;
	uint8_t wx_;
	uint8_t wy_;

	uint8_t bgp_;
};

}