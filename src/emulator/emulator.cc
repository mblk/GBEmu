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
	:emulatorData_(new EmulatorData(logFileName, romFileName, debugBitmap, displayBitmap, soundDevice),
		[](EmulatorData *ed) { delete ed; }),
	prevTime_(std::chrono::high_resolution_clock::now()),
	statPrevTime_(std::chrono::high_resolution_clock::now()),
	statTicks_(0)
{
}

void Emulator::Tick(const KeypadKeys &keys)
{
	const int numberOfCpuCyclesPerBatch = 4;

	const double targetTicksPerSecond = 4194304.0; // 4.194304MHz
	const double targetTicksDuration = 1.0 / targetTicksPerSecond; // ~238ns

	const int ticksPerFrame = 4194304 / 60;

	// Run emulator ticks.
	int totalTicks = 0;
	{
		while(totalTicks < ticksPerFrame)
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
	}

	// Slow down to match the correct CPU speed.
	#if 1
	for (;;)
	{
		const auto now = std::chrono::high_resolution_clock::now();
		const auto currentDt = std::chrono::duration<double, std::nano>(now - prevTime_);
		const auto targetDt = std::chrono::duration<double, std::nano>(
			static_cast<double>(totalTicks) * targetTicksDuration * 1000.0 * 1000.0 * 1000.0);

		if (currentDt > targetDt)
		{
			// Must use long long here as that's what high_resolution_clock is using internally.
			// Maybe there's a cleaner way to do this.
			prevTime_ = now - std::chrono::nanoseconds(static_cast<long long>((currentDt - targetDt).count()));
			break;
		}
	}
	#endif

	// Stats
	{
		const auto now = std::chrono::high_resolution_clock::now();
		const auto dt = std::chrono::duration<double>(now - statPrevTime_);

		double dtSeconds = dt.count();

		statTicks_ += totalTicks;
		
		if (dtSeconds > 2.5)
		{
			const double ticksPerSecond = double(statTicks_) / dtSeconds;
			const double absError = ticksPerSecond - targetTicksPerSecond;
			const double relError = absError / targetTicksPerSecond;

			printf("dt %.3lf ticks %d tps %.3lf absError %.3lf relError %.3lf%%\n", dtSeconds, statTicks_, ticksPerSecond, absError, relError*100.0);

			statPrevTime_ = now;
			statTicks_ = 0;
		}
	}
}

}