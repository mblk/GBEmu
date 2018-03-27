#pragma once

#include <cstdint>

#include <map>
#include <vector>

namespace GBEmu::Emulator
{

class Log;

class MemoryRegion
{
public:
	virtual uint16_t GetSize() const = 0;
	virtual uint8_t Read(uint16_t offset) = 0;
	virtual void Write(uint16_t offset, uint8_t data) = 0;

	void SetBase(uint16_t base) { base_ = base; }
	uint16_t GetBase() const { return base_; }

private:
	uint16_t base_;
};

class Memory
{
public:
	Memory(Log &log);
	virtual ~Memory();

	void Register(MemoryRegion *region, uint16_t base);

	uint8_t Read(uint16_t address);
	void Write(uint16_t address, uint8_t data);

private:
	MemoryRegion * LookupRegion(uint16_t address);

private:
	Log & log_;
	std::vector<MemoryRegion*> regions_;
};

}