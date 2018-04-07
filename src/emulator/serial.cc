#include "serial.hh"
#include "io.hh"
#include "log.hh"

namespace GBEmu::Emulator
{

Serial::Serial(Log &log, IO &io)
	:log_(log)
{
	io.Register("SB", 0x01, []() {
		return 0;
	}, [&](uint8_t v) {

		if (v >= 0x20 && v <= 0x7F)
		{
			s_ += reinterpret_cast<char&>(v);
			log_.Peripheral("Serial: '" + s_ + "'");
		}

	});

	io.Register("SC", 0x02, []() {
		return 0;
	}, [](uint8_t v) {
	});
}

void Serial::Tick()
{
}

}