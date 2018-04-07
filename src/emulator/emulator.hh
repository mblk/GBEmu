#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <array>

#include "keypad.hh"

namespace GBEmu::Emulator
{

class DisplayBitmap;
class SoundDevice;

class Emulator
{
public:
	Emulator(
		const std::string &logFileName,
		const std::string &romFileName,
		DisplayBitmap * const debugBitmap,
		DisplayBitmap &displayBitmap,
		SoundDevice &soundDevice);

	void Tick(const KeypadKeys &keys);

private:
	struct EmulatorData;
	const std::unique_ptr<EmulatorData> emulatorData_;
	std::chrono::high_resolution_clock::time_point prevTime_;
};

}