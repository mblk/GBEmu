#pragma once

#include <cstdint>

#include "emulator/sound.hh"

namespace GBEmu
{

class SdlSound : public Emulator::SoundDevice
{
public:
	SdlSound();
	virtual ~SdlSound();

	virtual void SetFrequency1(int freq) override;
	virtual void SetVolume1(int volume) override;
	virtual void SetFrequency2(int freq) override;
	virtual void SetVolume2(int volume) override;
	virtual void SetFrequency3(int freq) override;
	virtual void SetVolume3(int volume) override;
	virtual void SetPattern3(uint8_t *pattern) override;
	virtual void SetPlayback3(bool playback) override;

private:
	uint32_t deviceId_;
};

}