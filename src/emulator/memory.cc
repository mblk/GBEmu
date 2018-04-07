#include "memory.hh"
#include "log.hh"

#include <cassert>
#include <cstdlib>

namespace GBEmu::Emulator
{

Memory::Memory(Log &log)
	:log_(log)
{

}

void Memory::Register(MemoryRegion *region, uint16_t base)
{
	assert(region);
	assert(region->GetSize() > 0);
	assert((uint32_t)base + (uint32_t)region->GetSize() <= 0x10000);

	// TODO check for overlapping regions.

	region->SetBase(base);
	regions_.push_back(region);
}

uint8_t Memory::Read(uint16_t address)
{
	if (address >= 0xFEA0 && address <= 0xFEFF) return 0; // Not usable

	if (address >= 0xE000 && address <= 0xFDFF) address -= 0x2000;

	auto *region = LookupRegion(address);

	if (region)
	{
		return region->Read(address - region->GetBase());
	}
	else
	{
		log_.Error("Memory read at " + AsHexString(address) + " not implemented");
		return 0;
	}
}

void Memory::Write(uint16_t address, uint8_t data)
{
	if (address >= 0xFEA0 && address <= 0xFEFF) return; // Not usable

	if (address >= 0xE000 && address <= 0xFDFF) address -= 0x2000;

	auto *region = LookupRegion(address);
	
	if (region)
	{
		region->Write(address - region->GetBase(), data);
	}
	else
	{
		log_.Error("Memory write at " + AsHexString(address) + " (" + AsHexString(data) + ") not implemented");
	}
}

MemoryRegion *Memory::LookupRegion(uint16_t address)
{
	for (auto region : regions_)
	{
		auto base = region->GetBase();
		auto size = region->GetSize();

		if (address >= base && (uint32_t)address < (uint32_t)base + (uint32_t)size)
		{
			return region;
		}
	}

	return nullptr;
}

}