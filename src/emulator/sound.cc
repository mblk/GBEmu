#include "sound.hh"
#include "io.hh"

#include <cstdio>
#include <cassert>
#include <cstdint>

namespace GBEmu::Emulator
{

Sound::Sound(IO &io, SoundDevice &soundDevice)
	:io_(io),
	soundDevice_(soundDevice),

	nr10_(0),
	nr11_(0),
	nr12_(0),
	nr13_(0),
	nr14_(0),

	nr21_(0),
	nr22_(0),
	nr23_(0),
	nr24_(0),

	nr30_(0),
	nr31_(0),
	nr32_(0),
	nr33_(0),
	nr34_(0),

	c1ticks_(0),
	c1freq_(0),
	c1volume_(0),
	c1direction_(0),
	c1numberOfSweep_(0),

	c2ticks_(0),
	c2freq_(0),
	c2volume_(0),
	c2direction_(0),
	c2numberOfSweep_(0)

{
	for (int i = 0; i < 16; i++) pattern_[0] = 0;

	// Sound Channel 1
	io_.Register("NR10", 0x10, [&]() { return nr10_; }, [&](uint8_t v) {
		nr10_ = v;
	});
	io_.Register("NR11", 0x11, [&]() { return nr11_; }, [&](uint8_t v) {
		nr11_ = v;
	});
	io_.Register("NR12", 0x12, [&]() { return nr12_; }, [&](uint8_t v) {
		nr12_ = v;

		c1volume_ = (nr12_ & 0xF0) >> 4;
		c1direction_ = (nr12_ & 0x08) >> 3;
		c1numberOfSweep_ = nr12_ & 0x07;
	});
	io_.Register("NR13", 0x13, [&]() { return nr13_; }, [&](uint8_t v) {
		nr13_ = v;
	});
	io_.Register("NR14", 0x14, [&]() { return nr14_; }, [&](uint8_t v) {
		nr14_ = v;

		if (nr14_ & 0x80) // Initial?
		{
			c1ticks_ = 0;

			int x = ((nr14_ & 0x7) << 8) | nr13_;
			int freq = 131072 / (2048 - x);

			soundDevice_.SetFrequency1(freq);
			soundDevice_.SetVolume1(c1volume_);
		}
	});

	// Sound Channel 2
	io_.Register("NR21", 0x16, [&]() { return nr21_; }, [&](uint8_t v) {
		nr21_ = v;
	});
	io_.Register("NR22", 0x17, [&]() { return nr22_; }, [&](uint8_t v) {
		nr22_ = v;

		c2volume_ = (nr22_ & 0xF0) >> 4;
		c2direction_ = (nr22_ & 0x08) >> 3;
		c2numberOfSweep_ = nr22_ & 0x07;
	});
	io_.Register("NR23", 0x18, [&]() { return nr23_; }, [&](uint8_t v) {
		nr23_ = v;
	});
	io_.Register("NR24", 0x19, [&]() { return nr24_; }, [&](uint8_t v) {
		nr24_ = v;

		if (nr24_ & 0x80) // Initial?
		{
			c2ticks_ = 0;
			
			int x = ((nr24_ & 0x7) << 8) | nr23_;
			int freq = 131072 / (2048 - x);

			soundDevice_.SetFrequency2(freq);
			soundDevice_.SetVolume2(c1volume_);
		}
	});

	// Sound Channel 3
	io_.Register("NR30", 0x1A, [&]() { return nr30_; }, [&](uint8_t v) {
		nr30_ = v;

		soundDevice_.SetPlayback3((nr30_ & 0x80) != 0);
	});
	io_.Register("NR31", 0x1B, [&]() { return nr31_; }, [&](uint8_t v) {
		nr31_ = v;
	});
	io_.Register("NR32", 0x1C, [&]() { return nr32_; }, [&](uint8_t v) {
		nr32_ = v;
	});
	io_.Register("NR33", 0x1D, [&]() { return nr33_; }, [&](uint8_t v) {
		nr33_ = v;
	});
	io_.Register("NR34", 0x1E, [&]() { return nr34_; }, [&](uint8_t v) {
		nr34_ = v;

		if (nr34_ & 0x80)
		{
			int x = ((nr34_ & 0x7) << 8) | nr33_;
			int freq = 65536 / (2048 - x);
			int volume = (nr32_ & 0x60) >> 5;

			soundDevice_.SetFrequency3(freq);
			soundDevice_.SetVolume3(volume);
			soundDevice_.SetPattern3(pattern_);
		}

	});
	for (int i = 0; i <= 0xF; i++) {
		io_.Register("Wave Pattern Ram", 0x30 + i, [&, i]() { return pattern_[i]; }, [&, i](uint8_t v) {
			pattern_[i] = v;

		});
	}

	// Sound Channel 4
	io_.Register("NR41", 0x20, []() { return 0; }, [](uint8_t v) {});
	io_.Register("NR42", 0x21, []() { return 0; }, [](uint8_t v) {});
	io_.Register("NR43", 0x22, []() { return 0; }, [](uint8_t v) {});
	io_.Register("NR44", 0x23, []() { return 0; }, [](uint8_t v) {});

	// Sound Control
	io_.Register("NR50", 0x24, []() { return 0; }, [](uint8_t v) {});
	io_.Register("NR51", 0x25, []() { return 0; }, [](uint8_t v) {});
	io_.Register("NR52", 0x26, []() { return 0; }, [](uint8_t v) {});
}

Sound::~Sound()
{

}

void Sound::Tick(int consumedTicks)
{
	// CPU clock: 4.194304MHz
	const int c1TickOverflow = 4194304 / 64; // 64 Hz
	const int c2TickOverflow = 4194304 / 64; // 64 Hz


	if (c1numberOfSweep_)
	{
		c1ticks_ += consumedTicks;
		if (c1ticks_ > c1TickOverflow * c1numberOfSweep_)
		{
			c1ticks_ = 0;

			if (c1direction_) {
				if (c1volume_ < 15) c1volume_++;
			}
			else {
				if (c1volume_) c1volume_--;
			}

			soundDevice_.SetVolume1(c1volume_);
		}
	}


	if (c2numberOfSweep_)
	{
		c2ticks_ += consumedTicks;
		if (c2ticks_ > c2TickOverflow * c2numberOfSweep_)
		{
			c2ticks_ = 0;

			if (c2direction_) {
				if (c2volume_ < 15) c2volume_++;
			}
			else {
				if (c2volume_) c2volume_--;
			}

			soundDevice_.SetVolume2(c2volume_);
		}
	}

}

}