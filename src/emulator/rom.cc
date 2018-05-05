#include "rom.hh"
#include "log.hh"

#include <iostream>
#include <fstream>
#include <exception>

#include <cassert>


namespace GBEmu::Emulator
{

Rom::Rom(Log &log)
	:log_(log)
{
}

void Rom::Load(const std::string &filename)
{
	// Open file.
	std::ifstream stream(filename, std::ios_base::binary);

	if (!stream.is_open() || stream.bad())
	{
        return;
		//throw new std::exception("unable to open rom file");
	}
	
	// Get file size.
	auto startPosition = stream.tellg();
	stream.seekg(0, std::ios_base::end);
	auto endPosition = stream.tellg();
	stream.seekg(0, std::ios_base::beg);
	size_t fileSize = size_t(endPosition - startPosition);

	log_.Rom("File size: " + AsHexString(fileSize));

	assert(fileSize > 0);
	assert(!(fileSize % (32 * 1024)));

	// Read file content.
	data_.resize(fileSize);
	stream.read(reinterpret_cast<char*>(data_.data()), fileSize);

	cartridgeType_ = data_[0x147];
	romSize_ = data_[0x148];

	romBank_ = 0; //( ??

	log_.Rom("Cartridge type: " + AsHexString(cartridgeType_));
	log_.Rom("Size: " + AsHexString(romSize_));

	/*
  00h  ROM ONLY                 13h  MBC3+RAM+BATTERY
  01h  MBC1                     15h  MBC4
  02h  MBC1+RAM                 16h  MBC4+RAM
  03h  MBC1+RAM+BATTERY         17h  MBC4+RAM+BATTERY
  05h  MBC2                     19h  MBC5
  06h  MBC2+BATTERY             1Ah  MBC5+RAM
  08h  ROM+RAM                  1Bh  MBC5+RAM+BATTERY
  09h  ROM+RAM+BATTERY          1Ch  MBC5+RUMBLE
  0Bh  MMM01                    1Dh  MBC5+RUMBLE+RAM
  0Ch  MMM01+RAM                1Eh  MBC5+RUMBLE+RAM+BATTERY
  0Dh  MMM01+RAM+BATTERY        FCh  POCKET CAMERA
  0Fh  MBC3+TIMER+BATTERY       FDh  BANDAI TAMA5
  10h  MBC3+TIMER+RAM+BATTERY   FEh  HuC3
  11h  MBC3                     FFh  HuC1+RAM+BATTERY
  12h  MBC3+RAM

  00h -  32KByte (no ROM banking)
  01h -  64KByte (4 banks)
  02h - 128KByte (8 banks)
  03h - 256KByte (16 banks)
  04h - 512KByte (32 banks)
  05h -   1MByte (64 banks)  - only 63 banks used by MBC1
  06h -   2MByte (128 banks) - only 125 banks used by MBC1
  07h -   4MByte (256 banks)
  52h - 1.1MByte (72 banks)
  53h - 1.2MByte (80 banks)
  54h - 1.5MByte (96 banks)
	*/





	// Close file.
	stream.close();
}

uint8_t Rom::Read(uint16_t offset)
{
	assert(offset < size_);

	if (cartridgeType_ == 0x00) // None
	{
		return data_[offset];
	}
	else if (cartridgeType_ == 0x01 || cartridgeType_ == 0x02 || cartridgeType_ == 0x03) // MBC1
	{
		if (offset <= 0x3FFF)
		{
			return data_[offset];
		}
		else if (offset <= 0x7FFF)
		{
			size_t index = size_t(romBank_) * 16 * 1024 + offset - 0x4000;

			return data_[index];
		}
	}

	assert(0);
	return 0;
}

void Rom::Write(uint16_t offset, uint8_t data)
{
	assert(offset < size_);

	if (cartridgeType_ == 0x00) // None
	{
		//assert(0);
		log_.Error("Write to ROM at " + AsHexString(offset) + " (" + AsHexString(data) + ")");

	}
	else if (cartridgeType_ == 0x01 || cartridgeType_ == 0x02 || cartridgeType_ == 0x03) // MBC1
	{
		if (offset <= 0x1FFF) // RAM Enable
		{
			//assert(0);

			//printf("TODO Ram Enable\n");

		}
		else if (offset <= 0x3FFF) // ROM Bank Number
		{
			uint8_t bankNumber = data & 0x1F;

			if (bankNumber == 0) bankNumber = 1;
			
			romBank_ = bankNumber;

			log_.Rom("Selected ROM bank " + AsHexString(romBank_));
		}
		else if (offset <= 0x5FFF) // RAM Bank Number OR Upper Bits of ROM Bank
		{
			assert(0);
		}
		else if (offset <= 0x7FFF) // ROM/RAM Mode Select
		{
			assert(0);
		}
	}
	else
	{
		assert(0);
	}
}

}
