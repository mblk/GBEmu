#include "ram.hh"

#include <cassert>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace GBEmu::Emulator
{

Ram::Ram()
	:memory_({})
{
}

uint16_t Ram::GetSize() const
{
	return size_;
}

uint8_t Ram::Read(uint16_t offset)
{
	assert(offset < size_);
	return memory_[offset];
}

void Ram::Write(uint16_t offset, uint8_t data)
{
	assert(offset < size_);
	memory_[offset] = data;
	changed_ = true;
}

void Ram::Save(const std::string &filename)
{
	std::ofstream s;
	s.open(filename);

	const size_t bytesPerLine = 64;

	for (size_t line = 0; line < size_ / bytesPerLine; line++)
	{
		size_t offset = line * bytesPerLine;

		s << std::hex << std::setw(4) << std::setfill('0') << int(offset) << ":";

		for (size_t i = 0; i < bytesPerLine; i++)
		{
			s << std::hex << std::setw(2) << std::setfill('0') << int(memory_[offset + i]);
		}

		s << std::endl;
	}

	s.close();
}

}
