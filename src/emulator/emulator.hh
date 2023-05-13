#pragma once

#include <string>
#include <memory>

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
		size_t romSize, const void *romData,
		DisplayBitmap * const debugBitmap,
		DisplayBitmap &displayBitmap,
		SoundDevice &soundDevice);
	virtual ~Emulator();

	void Tick(double dt, const KeypadKeys &keys);

private:
	struct EmulatorData;
	const std::unique_ptr<EmulatorData> emulatorData_;

	bool statFirstCall_ = true;
	double statTime_ = 0;
	int statTicks_ = 0;
};

}