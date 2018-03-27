#pragma once

#define INT_VBLANK		(1 << 0)
#define INT_LCDC		(1 << 1)
#define INT_TIMER		(1 << 2)
#define INT_SERIAL		(1 << 3)
#define INT_PIN			(1 << 4)

#include <cstdint>

namespace GBEmu::Emulator
{

class Log;
class IO;

class Pic
{
public:
	Pic(Log &log, IO &io);
	virtual ~Pic();

	void RaiseInterrupts(uint8_t mask);
	uint8_t GetAndClearInterrupt();
	bool InterruptsPending() const;

private:
	Log & log_;
	IO & io_;

	uint8_t ie_;
	uint8_t if_;
};

}