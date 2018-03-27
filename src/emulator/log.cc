#include "log.hh"

#include <iostream>

namespace GBEmu::Emulator
{

Log::Log(const std::string &filename)
	:filename_(filename),
	instructionCount_(0),
	instructionFilter_(0),
	mask_(0)
{
	stream_.open(filename);

	stream_ << "Start" << std::endl;

	//instructionFilter_ = 1539100; // Test 03
	//instructionFilter_ = 3634579; // Test 04 somewhere

	
	mask_ = 0;
	//mask_ |= LOG_INSTRUCTION;
	//mask_ |= LOG_INTERRUPT;
	//mask_ |= LOG_STATE;
	//mask_ |= LOG_ROM;
	//mask_ |= LOG_MEMORY;
	//mask_ |= LOG_IO;
	//mask_ |= LOG_PERIPHERAL;
	//mask_ = 0xFF;
}

Log::~Log()
{
	stream_ << "End" << std::endl;
}

void Log::Instruction(const std::string &s)
{
	instructionCount_++;

	if (!InstructionEnabled()) return;
	if (instructionCount_ < instructionFilter_) return;

	stream_ << "INST " << s << " (" << instructionCount_ << ")" << std::endl;
	stream_.flush();
}

void Log::Interrupt(const std::string &s)
{
	if (!InterruptEnabled()) return;
	if (instructionCount_ < instructionFilter_) return;

	stream_ << "INT  " << s << std::endl;
	stream_.flush();
}

void Log::State(const std::string &s)
{
	if (!StateEnabled()) return;
	if (instructionCount_ < instructionFilter_) return;

	stream_ << "     " << s << std::endl;
	stream_.flush();
}

void Log::Rom(const std::string &s)
{
	if (!RomEnabled()) return;
	if (instructionCount_ < instructionFilter_) return;

	stream_ << "ROM  " << s << std::endl;
	stream_.flush();
}

void Log::Memory(const std::string &s)
{
	if (!MemoryEnabled()) return;
	if (instructionCount_ < instructionFilter_) return;

	stream_ << "MEM  " << s << std::endl;
	stream_.flush();
}

void Log::InputOutput(const std::string &s)
{
	if (!InputOutputEnabled()) return;
	if (instructionCount_ < instructionFilter_) return;

	stream_ << "IO   " << s << std::endl;
	stream_.flush();
}

void Log::Peripheral(const std::string &s)
{
	if (!PeripheralEnabled()) return;
	if (instructionCount_ < instructionFilter_) return;

	stream_ << "PERI " << s << std::endl;
	stream_.flush();
}

void Log::Error(const std::string &s)
{
	stream_ << "ERR  " << s << std::endl;
	stream_.flush();
}

}