#include "io.hh"
#include "log.hh"

#include <cstdlib>
#include <cassert>

namespace GBEmu::Emulator
{

IO::IO(Log &log)
	:log_(log),
	ports_({}),
	ram_({})
{
	// Sound Channel 1
	Register("?", 0x10, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x11, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x12, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x13, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x14, []() { return 0; }, [](uint8_t v) { });
	// Sound Channel 2
	Register("?", 0x16, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x17, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x18, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x19, []() { return 0; }, [](uint8_t v) { });
	// Sound Channel 3
	Register("?", 0x1A, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x1B, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x1C, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x1D, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x1E, []() { return 0; }, [](uint8_t v) { });
	for(uint8_t i=0x30; i<=0x3F; i++)
		Register("?", i, []() { return 0; }, [](uint8_t v) { });
	// Sound Channel 4
	Register("?", 0x20, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x21, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x22, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x23, []() { return 0; }, [](uint8_t v) { });
	// Sound Control
	Register("?", 0x24, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x25, []() { return 0; }, [](uint8_t v) { });
	Register("?", 0x26, []() { return 0; }, [](uint8_t v) { });


	// Joypad
	Register("JOYP", 0x00, [&]() {
		uint8_t r = 0xF;

		if (buttonKeys_)
		{
			if (start_) r &= ~8u;
			if (select_) r &= ~4u;
			if (b_) r &= ~2u;
			if (a_) r &= ~1u;
		}
		if (directionKeys_)
		{
			if (down_) r &= ~8u;
			if (up_) r &= ~4u;
			if (left_) r &= ~2u;
			if (right_) r &= ~1u;
		}

		return r;
	}, [&](uint8_t v) {

		if (!(v & 0x20)) buttonKeys_ = true; else buttonKeys_ = false;
		if (!(v & 0x10)) directionKeys_ = true; else directionKeys_ = false;
	});

	// KEY1 - CGB Mode Only - Prepare Speed Switch
	Register("KEY1", 0x4D, []() { return 0x7E; }, [](uint8_t v) { });
}

IO::~IO()
{
}

uint8_t IO::Read(uint16_t offset)
{
	assert(offset < size_);

	if (offset >= 0x80 && offset <= 0xFE) // High Ram
	{
		uint8_t data = ram_[offset];

		if (log_.MemoryEnabled())
			log_.Memory("Read from " + AsHexString(offset) + " -> " + AsHexString(data));

		return data;
	}
	else if (ports_[offset].Read)
	{
		uint8_t data = ports_[offset].Read();

		if (log_.InputOutputEnabled())
			log_.InputOutput("Read from " + AsHexString(offset) + " (" + ports_[offset].name + ") -> " + AsHexString(data));

		return data;
	}
	else
	{
		log_.Error("IO read at " + AsHexString(offset) + " not implemented");
		return 0;
	}
}

void IO::Write(uint16_t offset, uint8_t data)
{
	assert(offset < size_);

	if (offset >= 0x80 && offset <= 0xFE) // High Ram
	{
		if (log_.MemoryEnabled())
			log_.Memory("Write to " + AsHexString(offset) + " <- " + AsHexString(data));

		ram_[offset] = data;
	}
	else if (ports_[offset].Write)
	{
		if (log_.InputOutputEnabled())
			log_.InputOutput("Write to " + AsHexString(offset) + " (" + ports_[offset].name + ") <- " + AsHexString(data));

		ports_[offset].Write(data);
	}
	else
	{
		log_.Error("IO write at " + AsHexString(offset) + " (" + AsHexString(data) + ") not implemented");
	}
}

void IO::Register(const std::string &name, uint8_t offset, IOReadHandler read, IOWriteHandler write) // TODO std::move etc?
{
	assert(!ports_[offset].Read);
	assert(!ports_[offset].Write);

	ports_[offset].name = name;
	ports_[offset].Read = read;
	ports_[offset].Write = write;
}

}