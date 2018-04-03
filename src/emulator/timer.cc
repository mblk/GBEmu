#include "timer.hh"
#include "io.hh"
#include "pic.hh"
#include "log.hh"

#include <cstdio>

#include <iostream>
#include <fstream>

namespace GBEmu::Emulator
{

Timer::Timer(Log &log, IO &io, Pic &pic)
	:log_(log),
	io_(io),
	pic_(pic),
	divValue_(0),
	timaValue_(0),
	tmaValue_(0),
	tacValue_(0),
	divTicks_(0),
	timaTicks_(0)
{
	// DIV - Divider Register
	io.Register("DIV", 0x04, [&]() {
		return divValue_;
	}, [&](uint8_t v) {
		divValue_ = 0;
	});

	// TIMA - Timer Counter
	io.Register("TIMA", 0x05, [&]() {
		return timaValue_;
	}, [&](uint8_t v) {
		timaValue_ = v;
	});

	// TMA - Timer Modulo
	io.Register("TMA", 0x06, [&]() {
		return tmaValue_;
	}, [&](uint8_t v) {
		tmaValue_ = v;
	});

	// TAC - Timer Control
	io.Register("TAC", 0x07, [&]() {
		return tacValue_;
	}, [&](uint8_t v) {
		tacValue_ = v & 0x07;
	});
}

Timer::~Timer()
{
}

void Timer::Tick(int ticksPassed)
{
	// CPU clock: 4.194304MHz
	// x / 16 -> 262144 Hz
	// x / 64 -> 65536 Hz
	// x / 256 -> 16384 Hz
	// x / 1024 -> 4096 Hz

	divTicks_ += ticksPassed;
	if (divTicks_ > 256) // 16384 Hz
	{
		divTicks_ = 0;
		divValue_++;
	}

	if (tacValue_ & 0x4)
	{
		int timaOverflow = 0;
		switch (tacValue_ & 0x3)
		{
		case 0: timaOverflow = 1024; break; // 4096 Hz
		case 1: timaOverflow = 16; break; // 262144 Hz
		case 2: timaOverflow = 64; break; // 65536 Hz
		case 3: timaOverflow = 256; break; // 16384 Hz
		}

		timaTicks_ += ticksPassed;

		while (timaTicks_ > timaOverflow)
		{
			timaTicks_ -= timaOverflow;
			timaValue_++;

			log_.Peripheral("TIMA " + AsHexString(timaValue_));

			if (!timaValue_)
			{
				timaValue_ = tmaValue_;

				// Request Int.
				pic_.RaiseInterrupts(INT_TIMER);
			}
		}
	}
}

}