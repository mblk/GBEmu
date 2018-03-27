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
	Display(IO &io, Pic &pic, Ram &vram, SpriteAttributeTable &oam, DisplayBitmap &bitmap);
	virtual ~Display();

	void Tick(uint32_t ticks);

private:
	void Refresh();
	void DrawLine(uint8_t y);

	void DrawBackgroundTile(uint8_t index, uint8_t x, uint8_t y);
	void DrawSpriteTile(uint8_t index, uint8_t x, uint8_t y, uint8_t flags);
	void DrawTile(uint16_t tileDataOffset, uint8_t destX, uint8_t destY, bool ignore0, bool flipX, bool flipY);

private:
	IO &io_;
	Pic &pic_;
	Ram &vram_;
	SpriteAttributeTable &oam_;
	DisplayBitmap &bitmap_;

	//uint32_t ticks_;
	uint32_t lyTicks_;

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

/*
	LCDC:
		Bit 7 - LCD Control Operation *
		0: Stop completely (no picture on screen)
		1: operation
		Bit 6 - Window Tile Map Display Select
		0: $9800-$9BFF
		1: $9C00-$9FFF
		Bit 5 - Window Display
		0: off
		1: on
		Bit 4 - BG & Window Tile Data Select
		0: $8800-$97FF
		1: $8000-$8FFF <- Same area as OBJ
		Bit 3 - BG Tile Map Display Select
		0: $9800-$9BFF
		1: $9C00-$9FFF
		Bit 2 - OBJ (Sprite) Size
		0: 8*8
		1: 8*16 (width*height)
		Bit 1 - OBJ (Sprite) Display
		0: off
		1: on
		Bit 0 - BG & Window Display
		0: off
		1: on
*/

}