#include "emulator.hh"
#include "log.hh"
#include "memory.hh"
#include "rom.hh"
#include "ram.hh"
#include "io.hh"
#include "display.hh"
#include "sound.hh"
#include "keypad.hh"
#include "pic.hh"
#include "serial.hh"
#include "dma.hh"
#include "timer.hh"
#include "cpu.hh"

#include <cassert>

#include <chrono>
#include <thread>

namespace GBEmu::Emulator
{

struct Emulator::EmulatorData
{
	EmulatorData(
		const std::string &logFileName,
		const std::string &romFileName,
		DisplayBitmap * const debugBitmap,
		DisplayBitmap &displayBitmap,
		SoundDevice &soundDevice)
		:log(logFileName),
		rom(log),
		io(log),
		memory(log),
		keypad(io),
		pic(log, io),
		display(io, pic, vram, oam, debugBitmap, displayBitmap),
		sound(io, soundDevice),
		serial(log, io),
		dma(io, memory),
		timer(log, io, pic),
		cpu(log, memory, io, pic)
	{
		rom.Load(romFileName);

		memory.Register(&rom, 0x0000);
		memory.Register(&vram, 0x8000);
		memory.Register(&extram, 0xA000);
		memory.Register(&ram, 0xC000);
		memory.Register(&oam, 0xFE00);
		memory.Register(&io, 0xFF00);
	}

	Log log;
	Rom rom;
	Ram ram;
	Ram vram;
	Ram extram;
	SpriteAttributeTable oam;
	IO io;
	Memory memory;
	Keypad keypad;
	Pic pic;
	Display display;
	Sound sound;
	Serial serial;
	Dma dma;
	Timer timer;
	Cpu cpu;
};

Emulator::Emulator(
	const std::string &logFileName,
	const std::string &romFileName,
	DisplayBitmap * const debugBitmap,
	DisplayBitmap &displayBitmap,
	SoundDevice &soundDevice)
	/*:emulatorData_(std::make_unique<EmulatorData>(
		logFileName, romFileName, debugBitmap,
		displayBitmap, soundDevice)),*/

	:emulatorData_(new EmulatorData(logFileName, romFileName, debugBitmap, displayBitmap, soundDevice),
		[](EmulatorData *ed) { delete ed; }),

	prevTime_(std::chrono::high_resolution_clock::now())
{
}

void Emulator::Tick(const KeypadKeys &keys)
{
	const int numberOfBatchesToSimulate = 4;
	const int numberOfCpuCyclesPerBatch = 4;
	const int totalNumberOfCycles = numberOfBatchesToSimulate * numberOfCpuCyclesPerBatch;

	// Run emulator ticks.
	int totalTicks = 0;
	for(int batch = 0; batch < numberOfBatchesToSimulate; batch++)
	{
		int batchTicks = 0;
		for (int cpuCycle = 0; cpuCycle < numberOfCpuCyclesPerBatch; cpuCycle++)
			batchTicks += emulatorData_->cpu.Tick();
		totalTicks += batchTicks;

		emulatorData_->keypad.SetKeys(keys);
		emulatorData_->display.Tick(batchTicks);
		emulatorData_->timer.Tick(batchTicks);
		emulatorData_->sound.Tick(batchTicks);
	}

	// Slow down to match the correct CPU speed.
	for (;;)
	{
		auto now = std::chrono::high_resolution_clock::now();

		// CPU clock: 4.194304MHz
		const long long nsPerTick = 1000000000 / 4194304; // ~238
		const long long targetTimeNs = nsPerTick * totalTicks;
		const auto deltaTime = now - prevTime_;
		const long long currentTimeNs = deltaTime.count();
		const long long timeToSleep = targetTimeNs - currentTimeNs;

		if (timeToSleep < 0) {
			prevTime_ = now;
			break;
		}
	}
}

}