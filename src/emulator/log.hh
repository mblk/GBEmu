#pragma once

#include <string>
#include <fstream>

#include <sstream>
#include <string>
#include <iomanip>

#define LOG_INSTRUCTION		(1 << 0)
#define LOG_INTERRUPT		(1 << 1)
#define LOG_STATE			(1 << 2)
#define LOG_ROM				(1 << 3)
#define LOG_MEMORY			(1 << 4)
#define LOG_IO				(1 << 5)
#define LOG_PERIPHERAL		(1 << 6)


namespace GBEmu::Emulator
{

template <typename T>
static std::string AsHexString(T i)
{
	std::stringstream ss;
	ss << std::hex << std::setfill('0') << std::setw(sizeof(T) * 2) << (unsigned int)i << "h";
	return ss.str();
}

class Log
{
public:
	Log(const std::string &filename);
	virtual ~Log();

	void Instruction(const std::string &s);
	void Interrupt(const std::string &s);
	void State(const std::string &s);
	void Rom(const std::string &s);
	void Memory(const std::string &s);
	void InputOutput(const std::string &s);
	void Peripheral(const std::string &s);
	void Error(const std::string &s);

	inline bool InstructionEnabled() const { return mask_ & LOG_INSTRUCTION; }
	inline bool InterruptEnabled() const { return mask_ & LOG_INTERRUPT; }
	inline bool StateEnabled() const { return mask_ & LOG_STATE; }
	inline bool RomEnabled() const { return mask_ & LOG_ROM; }
	inline bool MemoryEnabled() const { return mask_ & LOG_MEMORY; }
	inline bool InputOutputEnabled() const { return mask_ & LOG_IO; }
	inline bool PeripheralEnabled() const { return mask_ & LOG_PERIPHERAL; }

private:
	std::string filename_;
	std::ofstream stream_;
	unsigned int mask_;
	unsigned int instructionCount_;
	unsigned int instructionFilter_;
};

}