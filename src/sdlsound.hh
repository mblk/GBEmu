#pragma once

#include <cstdint>
#include <cstddef>

#include "emulator/sound.hh"

namespace GBEmu
{

template<class T, size_t size>
class SampleRingBuffer
{
public:
	void Push(T data) {
		buffer_[writePointer_] = data;
		writePointer_ = (writePointer_ + 1) % size;
	}

	T Get() {
		T data = buffer_[readPointer_];
		readPointer_ = (readPointer_ + 1) % size;
		return data;
	}

	size_t RemainingSpace() {
		return size - DataSize();
	}

	size_t DataSize() {
		return (readPointer_ <= writePointer_)
			? (writePointer_ - readPointer_)
			: (size - (readPointer_ - writePointer_));
	}

private:
	T buffer_[size] = {};
	size_t readPointer_ = 0;
	size_t writePointer_ = 0;
};

template<int SampleRate>
class SoundChannel
{
public:
	void SetFrequency(int frequency) {
		frequency_ = frequency;
		if (frequency_ < 1) frequency_ = 1;
		if (frequency_ > SampleRate / 2) frequency_ = SampleRate / 2;
	}

	void SetVolume(int volume) {
		volume_ = volume;
		if (volume_ < 0) volume_ = 0;
		if (volume_ > 15) volume_ = 15;
	}

	int8_t EmitSample() {
		const int cycleLength = SampleRate / frequency_;
		cycle_ = (cycle_ + 1) % cycleLength;

		if (volume_ <= 0)
			return 0;

		if (cycle_ < cycleLength / 2)
			return -volume_;
		else
			return volume_;
	}

private:
	int frequency_ = 1;
	int volume_ = 0;
	int cycle_ = 0;
};

template<int SampleRate>
class PatternSoundChannel
{
public:
	void SetFrequency(int frequency) {
		frequency_ = frequency;
		if (frequency_ < 1) frequency_ = 1;
		if (frequency_ > SampleRate / 2) frequency_ = SampleRate / 2;
	}

	void SetVolume(int volume) {
		volume_ = volume;
		if (volume_ < 0) volume_ = 0;
		if (volume_ > 15) volume_ = 15;
	}

	void SetPattern(uint8_t *pattern) { // TODO std::array
		for(int i=0; i<16; i++)
			pattern_[i] = pattern[i];
	}

	void SetPlayback(bool playback) {
		playback_ = playback;
	}

	int8_t EmitSample() {
		if (!playback_) {
			patternIndex_ = 0;
			cycle_ = 0;
			return 0;
		}

		int cycleLength = SampleRate / frequency_ / 32;
		if (cycleLength < 1) cycleLength = 1;
		cycle_ = (cycle_ + 1) % cycleLength;

		if (cycle_ == 0)
			patternIndex_ = (patternIndex_ + 1) % 32;

		uint8_t sample = pattern_[patternIndex_ / 2];
		if (patternIndex_ % 2 == 0)
			sample = sample >> 4;
		else
			sample = sample & 0xF;

		switch(volume_)
		{
			default: 
			case 0: sample = 0; break;
			case 1: break;
			case 2: sample >>= 1; break;
			case 3: sample >>= 2; break;
		}

		return static_cast<int8_t>(sample);
	}

private:
	int frequency_ = 1;
	int volume_ = 0;
	uint8_t pattern_[16] = {};
	int patternIndex_ = 0;
	bool playback_ = false;
	int cycle_ = 0;
};

class SdlSound : public Emulator::SoundDevice
{
public:
	SdlSound();
	virtual ~SdlSound();

	virtual void SetFrequency1(int freq)       override { channel1_.SetFrequency(freq);    }
	virtual void SetVolume1(int volume)        override { channel1_.SetVolume(volume);     }
	virtual void SetFrequency2(int freq)       override { channel2_.SetFrequency(freq);    }
	virtual void SetVolume2(int volume)        override { channel2_.SetVolume(volume);     }
	virtual void SetFrequency3(int freq)       override { channel3_.SetFrequency(freq);    }
	virtual void SetVolume3(int volume)        override { channel3_.SetVolume(volume);     }
	virtual void SetPattern3(uint8_t *pattern) override { channel3_.SetPattern(pattern);   }
	virtual void SetPlayback3(bool playback)   override { channel3_.SetPlayback(playback); }

	virtual void Tick(int consumedTicks) override;

private:
	int8_t EmitSample();

	static void SDL_AudioCallback(void *userdata, uint8_t *stream, int len);

private:
	constexpr static int SampleRate = 44100;
	constexpr static int SampleSize = 4096;
	//constexpr static int SampleSize = 16384;

	uint32_t deviceId_;
	int ticks_;

	SampleRingBuffer<int8_t, SampleSize * 4> sampleBuffer_;

	SoundChannel<SampleRate> channel1_;
	SoundChannel<SampleRate> channel2_;
	PatternSoundChannel<SampleRate> channel3_;
};

}