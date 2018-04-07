#pragma once

#include "memory.hh"

#include <cstdint>
#include <functional>
#include <array>
#include <string>

namespace GBEmu::Emulator
{

class Log;

using IOReadHandler = std::function<uint8_t()>;
using IOWriteHandler = std::function<void(uint8_t)>;

struct IOPort
{
	std::string name;
	IOReadHandler Read;
	IOWriteHandler Write;
};

class IO : public MemoryRegion
{
public:
	IO(Log &log);

	virtual uint16_t GetSize() const override { return size_; }
	virtual uint8_t Read(uint16_t offset) override;
	virtual void Write(uint16_t offset, uint8_t data) override;

	void Register(const std::string &name, uint8_t offset, IOReadHandler read, IOWriteHandler write);

private:
	Log & log_;

	static constexpr size_t size_ = 0x100; // 256
	std::array<IOPort, size_> ports_;
	std::array<uint8_t, size_> ram_;
};

}