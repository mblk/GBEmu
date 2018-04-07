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
	// KEY1 - CGB Mode Only - Prepare Speed Switch
	Register("KEY1", 0x4D, []() { return 0x7E; }, [](uint8_t v) { });
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