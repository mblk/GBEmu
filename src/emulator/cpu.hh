#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>
#include <functional>
#include <array>

#include <fstream>

namespace GBEmu::Emulator
{

class Log;
class Memory;
class IO;
class Pic;

struct Registers
{
	union {
		uint16_t af;
		struct {
			union {
				uint8_t f;
				struct {
					uint8_t flagX : 4;
					uint8_t flagC : 1;
					uint8_t flagH : 1;
					uint8_t flagN : 1;
					uint8_t flagZ : 1;
				};
			};
			uint8_t a;
		};
	};
	union {
		uint16_t bc;
		struct { uint8_t c, b; };
	};
	union {
		uint16_t de;
		struct { uint8_t e, d; };
	};
	union {
		uint16_t hl;
		struct { uint8_t l, h; };
	};
	union {
		uint16_t sp;
		struct { uint8_t spl, sph; };
	};
	union {
		uint16_t pc;
		struct { uint8_t pcl, pch; };
	};

	std::string ToString() const
	{
		std::stringstream ss;

		ss << "AF=" << std::hex << std::setw(4) << std::setfill('0') << af;
		ss << " BC=" << std::hex << std::setw(4) << bc;
		ss << " DE=" << std::hex << std::setw(4) << de;
		ss << " HL=" << std::hex << std::setw(4) << hl;
		ss << " SP=" << std::hex << std::setw(4) << sp;
		ss << " PC=" << std::hex << std::setw(4) << pc;

		return ss.str();
	}
};

static_assert(sizeof(Registers) == 12);

using InstructionHandler = std::function<void(uint8_t*)>;
using InstructionHandler2 = std::function<void(uint8_t, uint8_t*)>;

struct Instruction
{
	std::string name;
	uint8_t operands;
	uint8_t ticks;
	InstructionHandler handler;
	InstructionHandler2 handler2;
};

class Cpu
{
public:
	Cpu(Log &log, Memory &memory, IO &io, Pic &pic);

	void Reset();
	uint32_t Tick();

private:
	void Push8(uint8_t v);
	void Push16(uint16_t v);
	uint8_t Pop8();
	uint16_t Pop16();
	void Bitops(uint8_t opcode);

private:
	Log & log_;
	Memory & memory_;
	IO & io_;
	Pic & pic_;
	Registers regs_;
	bool interruptsEnabled_;
	bool halted_;

	std::array<Instruction, 256> instructions_;
	std::array<Instruction, 256> instructionsCB_;
	std::array<Instruction, 256> instructions10_;
};

}