#include "pic.hh"
#include "io.hh"
#include "log.hh"

#include <cstdlib>
#include <cassert>

namespace GBEmu::Emulator
{

Pic::Pic(Log &log, IO &io)
	:log_(log),
	io_(io),
	if_(0),
	ie_(0)
{
	// Interrupt Flags
	io_.Register("IF", 0x0F, [&]() {
		return if_;
	}, [&](uint8_t v) {
		if_ = v;
	});

	// Interrupt Enable
	io_.Register("IE", 0xFF, [&]() {
		return ie_;
	}, [&](uint8_t v) {
		ie_ = v;
	});
}

void Pic::RaiseInterrupts(uint8_t mask)
{
	log_.Interrupt("Raise " + AsHexString(mask));

	assert(!(mask & 0xE0));
	assert(mask & 0x1F);

	if_ |= mask;
}

uint8_t Pic::GetAndClearInterrupt()
{
	uint8_t r = if_ & ie_;

	if (r)
	{
		// Find set IF-bit with highest priority.
		for (int i = 7; i >= 0; i--)
		{
			uint8_t b = 1 << i;

			if (r & b)
			{
				// Clear highest priority bit.
				if_ &= ~b;

				log_.Interrupt("Get " + AsHexString(b) + " (IF=" + AsHexString(if_) + ")");
				return b;
			}
		}
	}

	return 0;
}

bool Pic::InterruptsPending() const
{
	return if_ != 0;
}

}