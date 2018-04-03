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

Emulator::Emulator(
	const std::string &logFileName,
	const std::string &romFileName,
	DisplayBitmap * const debugBitmap,
	DisplayBitmap &displayBitmap,
	SoundDevice &soundDevice)
{
	log_ = new Log(logFileName);

	rom_ = new Rom(*log_);
	rom_->Load(romFileName);
	vram_ = new Ram();
	extram_ = new Ram();
	ram_ = new Ram();
	oam_ = new SpriteAttributeTable();
	io_ = new IO(*log_);

	memory_ = new Memory(*log_);
	memory_->Register(rom_, 0x0000);
	memory_->Register(vram_, 0x8000);
	memory_->Register(extram_, 0xA000);
	memory_->Register(ram_, 0xC000);
	memory_->Register(oam_, 0xFE00);
	memory_->Register(io_, 0xFF00);

	keypad_ = new Keypad(*io_);
	pic_ = new Pic(*log_, *io_);
	display_ = new Display(*io_, *pic_, *vram_, *oam_, debugBitmap, displayBitmap);
	sound_ = new Sound(*io_, soundDevice);
	serial_ = new Serial(*log_, *io_);
	dma_ = new Dma(*io_, *memory_);
	timer_ = new Timer(*log_, *io_, *pic_);
	cpu_ = new Cpu(*log_, *memory_, *io_, *pic_);
}

Emulator::~Emulator()
{
}

void Emulator::Tick(bool keys[8])
{
	//const int cpuBatchSize = 64;
	const int cpuBatchSize = 16;

	const auto startTime = std::chrono::high_resolution_clock::now();

	// Run emulator ticks.
	int consumedTicks = 0;
	for (int i = 0; i < cpuBatchSize; i++)
		consumedTicks += cpu_->Tick();

	keypad_->SetKeys(keys);
	display_->Tick(consumedTicks);
	timer_->Tick(consumedTicks);
	sound_->Tick(consumedTicks);

	// Slow down to match the correct CPU speed.
	for (;;)
	{
		// CPU clock: 4.194304MHz
		//const long long nsPerTick = 1000000000 / 5594304; // ~238
		const long long nsPerTick = 1000000000 / 4194304; // ~238
		const long long targetTimeNs = nsPerTick * consumedTicks;
		const auto deltaTime = std::chrono::high_resolution_clock::now() - startTime;
		const long long currentTimeNs = deltaTime.count();
		const long long timeToSleep = targetTimeNs - currentTimeNs;

		if (timeToSleep < 0) break;
	}
}

}