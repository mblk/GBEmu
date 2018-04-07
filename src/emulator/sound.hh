#pragma once

#include <cstdint>

namespace GBEmu::Emulator
{

class IO;

class SoundDevice
{
public:
	virtual void SetFrequency1(int freq) = 0;
	virtual void SetVolume1(int volume) = 0;
	virtual void SetFrequency2(int freq) = 0;
	virtual void SetVolume2(int volume) = 0;
	virtual void SetFrequency3(int freq) = 0;
	virtual void SetVolume3(int volume) = 0;
	virtual void SetPattern3(uint8_t *pattern) = 0;
	virtual void SetPlayback3(bool playback) = 0;

};

class Sound
{
public:
	Sound(IO &io, SoundDevice &soundDevice);

	void Tick(int consumedTicks);

private:
	IO & io_;
	SoundDevice &soundDevice_;

	uint8_t nr10_;
	uint8_t nr11_;
	uint8_t nr12_;
	uint8_t nr13_;
	uint8_t nr14_;

	uint8_t nr21_;
	uint8_t nr22_;
	uint8_t nr23_;
	uint8_t nr24_;

	uint8_t nr30_;
	uint8_t nr31_;
	uint8_t nr32_;
	uint8_t nr33_;
	uint8_t nr34_;
	uint8_t pattern_[16];

	int c1ticks_;
	int c1freq_;
	int c1volume_;
	int c1direction_;
	int c1numberOfSweep_;

	int c2ticks_;
	int c2freq_;
	int c2volume_;
	int c2direction_;
	int c2numberOfSweep_;


};

}