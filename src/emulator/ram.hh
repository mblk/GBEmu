#pragma once

#include "memory.hh"

#include <cstdint>
#include <array>
#include <string>

namespace GBEmu::Emulator
{

class Ram : public MemoryRegion
{
public:
	Ram();
	virtual ~Ram();

	virtual uint16_t GetSize() const override;
	virtual uint8_t Read(uint16_t offset) override;
	virtual void Write(uint16_t offset, uint8_t data) override;

	void Save(const std::string &filename);

	bool Changed() {
		bool r = changed_;
		changed_ = false;
		return r;
	}

private:
	static constexpr size_t size_ = 8 * 1024;
	std::array<uint8_t, size_> memory_;
	bool changed_;
};

}