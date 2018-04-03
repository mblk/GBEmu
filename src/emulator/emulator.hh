#pragma once

#include <string>
#include <chrono>

namespace GBEmu::Emulator
{

class DisplayBitmap;
class SoundDevice;

class Log;
class Rom;
class Ram;
class SpriteAttributeTable;
class IO;
class Memory;

class Keypad;
class Pic;
class Display;
class Sound;
class Serial;
class Dma;
class Timer;
class Cpu;

class Emulator
{
public:
	Emulator(const std::string &logFileName,
		const std::string &romFileName,
		DisplayBitmap * const debugBitmap,
		DisplayBitmap &displayBitmap,
		SoundDevice &soundDevice);
	virtual ~Emulator();

	void Tick(bool keys[8]);

private:
	Log * log_;
	Rom *rom_;
	Ram *ram_;
	Ram *vram_;
	Ram *extram_;
	SpriteAttributeTable *oam_;
	IO *io_;
	Memory *memory_;
	Keypad *keypad_;
	Pic *pic_;
	Display *display_;
	Sound *sound_;
	Serial *serial_;
	Dma *dma_;
	Timer *timer_;
	Cpu *cpu_;

	std::chrono::high_resolution_clock::time_point prevTime_;



};

}