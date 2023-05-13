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

namespace GBEmu::Emulator
{

struct Emulator::EmulatorData
{
	EmulatorData(
		const std::string &logFileName,
		size_t romSize, const void *romData,
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
		rom.Load(romSize, romData);

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
	size_t romSize, const void *romData,
	DisplayBitmap * const debugBitmap,
	DisplayBitmap &displayBitmap,
	SoundDevice &soundDevice)
	:emulatorData_(
		std::make_unique<EmulatorData>(logFileName, romSize, romData, debugBitmap, displayBitmap, soundDevice)
	)
{
}

Emulator::~Emulator()
{
}

void Emulator::Tick(double dt, const KeypadKeys &keys)
{
	const double targetTicksPerSecond = 4194304.0; // 4.194304MHz CPU Clock
	const int targetTicksThisFrame = (int)(dt * targetTicksPerSecond); // 69905 @ 60 FPS
	const int cpuCyclesPerBatch = 4;

	// Run emulator ticks.
	int executedTicks = 0;
	{
		while(executedTicks < targetTicksThisFrame)
		{
			int batchTicks = 0;
			for (int cpuCycle = 0; cpuCycle < cpuCyclesPerBatch; cpuCycle++)
				batchTicks += emulatorData_->cpu.Tick();
			executedTicks += batchTicks;

			emulatorData_->keypad.SetKeys(keys);
			emulatorData_->display.Tick(batchTicks);
			emulatorData_->timer.Tick(batchTicks);
			emulatorData_->sound.Tick(batchTicks);
		}
	}

	// Stats
	{
		statTime_ += dt;
		statTicks_ += executedTicks;

		if (statTime_ > 2.5)
		{
			const double ticksPerSecond = double(statTicks_) / statTime_;
			const double absError = ticksPerSecond - targetTicksPerSecond;
			const double relError = absError / targetTicksPerSecond;

			printf("Emulation speed error %0.3f%%\n", relError);

			// printf("dt %.3lf ticks %d tps %.3lf absError %.3lf relError %.3lf%%\n",
			// 	statTime_, statTicks_, ticksPerSecond, absError, relError*100.0);

			statTime_ = 0;
			statTicks_ = 0;
		}
	}
}

}