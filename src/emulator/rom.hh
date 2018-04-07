#pragma once

#include "memory.hh"

#include <vector>
#include <cstdint>

namespace GBEmu::Emulator
{

class Log;

class Rom : public MemoryRegion
{
public:
	Rom(Log &log);

	void Load(const std::string &filename);

	virtual uint16_t GetSize() const override { return size_; }
	virtual uint8_t Read(uint16_t offset) override;
	virtual void Write(uint16_t offset, uint8_t data) override;

private:
	static constexpr size_t size_ = 32 * 1024;

	Log & log_;

	std::vector<uint8_t> data_;

	uint8_t cartridgeType_;
	uint8_t romSize_;

	uint8_t romBank_;
};

}