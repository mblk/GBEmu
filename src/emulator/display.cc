#include "display.hh"
#include "io.hh"
#include "pic.hh"
#include "ram.hh"

#include <cassert>

namespace GBEmu::Emulator
{

SpriteAttributeTable::SpriteAttributeTable()
	:table_({})
{
}

SpriteAttributeTable::~SpriteAttributeTable()
{
}

uint8_t SpriteAttributeTable::Read(uint16_t offset)
{
	assert(offset < size_);
	return table_[offset];
}

void SpriteAttributeTable::Write(uint16_t offset, uint8_t data)
{
	assert(offset < size_);
	table_[offset] = data;
}

Display::Display(IO &io, Pic &pic, Ram &vram, SpriteAttributeTable &oam, DisplayBitmap &debugBitmap, DisplayBitmap &displayBitmap)
	:io_(io),
	pic_(pic),
	vram_(vram),
	oam_(oam),
	debugBitmap_(debugBitmap),
	displayBitmap_(displayBitmap),	
	lyTicks_(0),
	lcdc_(0),
	lcds_(0),
	scx_(0),
	scy_(0),
	ly_(0),
	lyc_(0),
	wx_(0),
	wy_(0),
	bgp_(0)
{
	// LCD Control
	io_.Register("LCDC", 0x40, [&]() { return lcdc_; }, [&](uint8_t v) { lcdc_ = v; });

	// LCD Status
	io_.Register("LCDS", 0x41, [&]() { return lcds_; }, [&](uint8_t v) {
		lcds_ = (v & 0x78) | (lcds_ & 0x7);
	});

	// Monochrome Palettes
	io_.Register("BGP", 0x47, [&]() { return bgp_; }, [&](uint8_t v) { bgp_ = v; });
	io_.Register("OBP0", 0x48, []() { return 0; }, [](uint8_t v) { });
	io_.Register("OBP1", 0x49, []() { return 0; }, [](uint8_t v) { });

	// LCD Position and Scrolling
	io_.Register("SCY", 0x42, [&]() { return scy_; }, [&](uint8_t v) { scy_ = v; });
	io_.Register("SCX", 0x43, [&]() { return scx_; }, [&](uint8_t v) { scx_ = v; });
	io_.Register("LY", 0x44, [&]() { return ly_++; }, [&](uint8_t v) { ly_ = 0; });
	io_.Register("LYC", 0x45, [&]() { return lyc_; }, [&](uint8_t v) { lyc_ = v; });
	io_.Register("WY", 0x4A, [&]() { return wy_; }, [&](uint8_t v) { wy_ = v; });
	io_.Register("WX", 0x4B, [&]() { return wx_; }, [&](uint8_t v) { wx_ = v; });
}

Display::~Display()
{
}

void Display::Tick(uint32_t ticks)
{
	// CPU clock: 4.194304MHz
	// LY 00..144..153(vblank)
	// 60 * 154 = 9240
	// block / 9240 = 453

	// The LY can take on any value between 0 through 153. The values between 144 and 153 indicate the V-Blank period. Writing will reset the counter.

	const uint32_t lyTicksMax = 453; // ~60Hz * 154

	lyTicks_ += ticks;

	while (lyTicks_ > lyTicksMax)
	{
		lyTicks_ -= lyTicksMax;

		// Increase LY.
		ly_++;
		if (ly_ == 154) {
			ly_ = 0;
			RefreshDebugBitmap();
		}

		// Update display.
		if (ly_ == 0) {
			displayBitmap_.Clear();
		}
		DrawLine(ly_);
		if (ly_ == 144) {
			displayBitmap_.Present();
		}

		// Raise VBLANK interrupt?
		if (ly_ == 144) {
			pic_.RaiseInterrupts(INT_VBLANK);
		}

		// Raise Coincidence interrupt?
		if (ly_ == lyc_)
		{
			lcds_ |= 0x4; // Set Coincidence Flag

			if (lcds_ & 0x40) {
				pic_.RaiseInterrupts(INT_LCDC);
			}
		}
		else
		{
			lcds_ &= ~0x4; // Clear Coincidence Flag
		}
	}
}

void Display::DrawLine(uint8_t y)
{
	if (y > 143) return;

	// Background
	if (lcdc_ & 0x01)
	{
		const uint16_t backgroundTileMapBase = (lcdc_ & 0x08) ? 0x1C00 : 0x1800;

		uint8_t tileY = y / 8;
		uint8_t lineOffsetY = y % 8;

		for (uint8_t tileX = 0; tileX < 32; tileX++) {

			uint8_t tile = vram_.Read(backgroundTileMapBase + tileY * 32 + tileX);

			DrawBackgroundTileLine(tile, tileX * 8, tileY * 8, lineOffsetY);
		}
	}

	// Sprites
	if (lcdc_ & 0x02)
	{
		// Size: lcdc_ & 0x04
		// 0 -> 8x8
		// 1 -> 8x16
		assert(!(lcdc_ & 0x4));

		for (uint16_t index = 0; index < 40; index++)
		{
			uint8_t d[4];

			for (uint16_t i = 0; i<4; i++)
				d[i] = oam_.Read(index * 4 + i);

			uint8_t spriteY = d[0] - 16u;
			uint8_t spriteX = d[1] - 8u;
			uint8_t spriteIndex = d[2];
			uint8_t spriteFlags = d[3];

			// Off screen?
			//if (spriteX >= 144u) continue;
			if (y - spriteY >= 8u) continue;

			uint8_t lineOffsetY = y - spriteY;
			assert(lineOffsetY < 8u);

			DrawSpriteTileLine(spriteIndex, spriteX, spriteY, spriteFlags, lineOffsetY);
		}
	}
}

void Display::DrawBackgroundTileLine(uint8_t index, uint8_t x, uint8_t y, uint8_t lineOffsetY)
{
	uint16_t tileDataOffset = 0;

	if (lcdc_ & 0x10)
	{
		tileDataOffset += index * 16;
	}
	else
	{
		//tileDataOffset = 0x0800;
		tileDataOffset = 0x1000;
		int8_t signedIndex = reinterpret_cast<int8_t&>(index);
		tileDataOffset += signedIndex * 16;
	}

	DrawTileLine(tileDataOffset, x, y, lineOffsetY, false, false, false, true, true);
}

void Display::DrawSpriteTileLine(uint8_t index, uint8_t x, uint8_t y, uint8_t flags, uint8_t lineOffsetY)
{
	uint16_t tileDataOffset = index * 16;

	bool flipX = flags & 0x20;
	bool flipY = flags & 0x40;

	// TODO palette number.

	DrawTileLine(tileDataOffset, x, y, lineOffsetY, true, flipX, flipY, false, false);
}

void Display::DrawTileLine(uint16_t tileDataOffset, uint8_t destX, uint8_t destY, uint8_t lineOffsetY, bool ignore0, bool flipX, bool flipY, bool scrollX, bool scrollY)
{
	uint8_t tileData[16];
	for (size_t i = 0; i < 16; i++)
		tileData[i] = vram_.Read(tileDataOffset + i);

	uint8_t colors[4];
	colors[3] = bgp_ >> 6;
	colors[2] = (bgp_ & 0x30) >> 4;
	colors[1] = (bgp_ & 0x0C) >> 2;
	colors[0] = (bgp_ & 0x03);

	uint8_t y = lineOffsetY;

	// TODO scy
	assert(!scy_);

	for (uint8_t x = 0; x < 8; x++)
	{
		uint8_t targetX = destX + x;

		if (scrollX) targetX -= scx_;

		if (targetX >= 160) continue;

		uint8_t c = 0;

		if (flipX)
		{
			if (tileData[y * 2 + 0] & (0x01 << x)) c += 1;
			if (tileData[y * 2 + 1] & (0x01 << x)) c += 2;
		}
		else
		{
			if (tileData[y * 2 + 0] & (0x80 >> x)) c += 1;
			if (tileData[y * 2 + 1] & (0x80 >> x)) c += 2;
		}

		// Must ignore color=0 for sprites.
		if (!c && ignore0) continue;

		uint8_t brightness = 0;
		switch (colors[c])
		{
		case 0: brightness = 255; break;
		case 1: brightness = 170; break;
		case 2: brightness = 85; break;
		case 3: brightness = 0; break;
		}

		displayBitmap_.DrawPixel(targetX, destY + y, brightness);
	}
}






void Display::RefreshDebugBitmap()
{
	debugBitmap_.Clear();

	// Background
	if (lcdc_ & 0x01)
	{
		const uint16_t backgroundTileMapBase = (lcdc_ & 0x08) ? 0x1C00 : 0x1800;

		for (uint8_t tileY = 0; tileY < 32; tileY++) {
			for (uint8_t tileX = 0; tileX < 32; tileX++) {

				uint8_t tile = vram_.Read(backgroundTileMapBase + tileY * 32 + tileX);

				DrawBackgroundTile(tile, tileX * 8, tileY * 8);
			}
		}
	}

	// Sprites
	if (lcdc_ & 0x02)
	{
		// Size: lcdc_ & 0x04
		// 0 -> 8x8
		// 1 -> 8x16
		assert(!(lcdc_ & 0x4));
		
		for (uint16_t index = 0; index < 40; index++)
		{
			uint8_t d[4];

			for(uint16_t i=0; i<4; i++)
				d[i] = oam_.Read(index * 4 + i);

			int y = d[0] - 16;
			int x = d[1] - 8;
			if (x < 0 || y < 0) continue; // offscreen?

			DrawSpriteTile(d[2], (uint8_t)x, (uint8_t)y, d[3]);
		}
	}

	debugBitmap_.Present();
}

void Display::DrawBackgroundTile(uint8_t index, uint8_t x, uint8_t y)
{
	uint16_t tileDataOffset = 0;

	if (lcdc_ & 0x10)
	{
		tileDataOffset += index * 16;
	}
	else
	{
		//tileDataOffset = 0x0800;
		tileDataOffset = 0x1000;
		int8_t signedIndex = reinterpret_cast<int8_t&>(index);
		tileDataOffset += signedIndex * 16;
	}

	DrawTile(tileDataOffset, x, y, false, false, false);
}

void Display::DrawSpriteTile(uint8_t index, uint8_t x, uint8_t y, uint8_t flags)
{
	uint16_t tileDataOffset = index * 16;

	bool flipX = flags & 0x20;
	bool flipY = flags & 0x40;

	// TODO palette number.

	DrawTile(tileDataOffset, x, y, true, flipX, flipY);
}

void Display::DrawTile(uint16_t tileDataOffset, uint8_t destX, uint8_t destY, bool ignore0, bool flipX, bool flipY)
{
	uint8_t tileData[16];
	for (size_t i = 0; i < 16; i++)
		tileData[i] = vram_.Read(tileDataOffset + i);

	uint8_t colors[4];
	colors[3] = bgp_ >> 6;
	colors[2] = (bgp_ & 0x30) >> 4;
	colors[1] = (bgp_ & 0x0C) >> 2;
	colors[0] = (bgp_ & 0x03);

	for (uint8_t y = 0; y < 8; y++)
	{
		for (uint8_t x = 0; x < 8; x++)
		{
			uint8_t c = 0;

			if (flipX)
			{
				if (tileData[y * 2 + 0] & (0x01 << x)) c += 1;
				if (tileData[y * 2 + 1] & (0x01 << x)) c += 2;
			}
			else
			{
				if (tileData[y * 2 + 0] & (0x80 >> x)) c += 1;
				if (tileData[y * 2 + 1] & (0x80 >> x)) c += 2;
			}

			// Must ignore color=0 for sprites.
			if (!c && ignore0) continue;

			uint8_t brightness = 0;
			switch (colors[c])
			{
			case 0: brightness = 255; break;
			case 1: brightness = 170; break;
			case 2: brightness = 85; break;
			case 3: brightness = 0; break;
			}

			debugBitmap_.DrawPixel(destX + x, destY + y, brightness);
		}
	}
}

}