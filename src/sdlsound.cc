#include "sdlsound.hh"

#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstring>

#include <SDL.h>
#undef main

namespace GBEmu
{

//const int SampleRate = 11025; // Samples per second
//const int SampleRate = 22050; // Samples per second
const int SampleRate = 44100; // Samples per second

//const int SampleSize = 4096;
const int SampleSize = 256;

int TargetFreq1 = 150;
int TargetVolume1 = 0;
int TargetFreq2 = 150;
int TargetVolume2 = 0;
int TargetFreq3 = 150;
int TargetVolume3 = 0;
uint8_t Pattern3[16];
int Pattern3Index = 0;
bool Playback3 = false;

int CurrentCycleIndex1 = 0;
int CurrentCycleIndex2 = 0;
int CurrentCycleIndex3 = 0;

void SDL_AudioCallback(void *userdata, uint8_t* rawStream, int len)
{
	//printf("pattern: ");
	//for (int i = 0; i < 16; i++) printf("%02X ", Pattern3[i]);
	//printf("\n");

	int CycleLength1 = SampleRate / TargetFreq1;
	int CycleLength2 = SampleRate / TargetFreq2;
	int CycleLength3 = SampleRate / TargetFreq3 / 32;

	//printf("callback %p %d\n", stream, len);

	int8_t *stream = reinterpret_cast<int8_t*>(rawStream);

	for (int i = 0; i < len; )
	{
		CurrentCycleIndex1++;
		if (CurrentCycleIndex1 > CycleLength1) CurrentCycleIndex1 = 0;
		CurrentCycleIndex2++;
		if (CurrentCycleIndex2 > CycleLength2) CurrentCycleIndex2 = 0;
		CurrentCycleIndex3++;
		if (CurrentCycleIndex3 > CycleLength3) CurrentCycleIndex3 = 0;

		int s = 0;

#if 1
		if (CurrentCycleIndex1 < CycleLength1 / 2)
			s -= TargetVolume1;
		else
			s += TargetVolume1;
#endif

#if 1
		if (CurrentCycleIndex2 < CycleLength2 / 2)
			s -= TargetVolume2;
		else
			s += TargetVolume2;
#endif

#if 1
		if (Playback3)
		{
			uint8_t us = Pattern3[Pattern3Index / 2];
			if (Pattern3Index % 2 == 0) us = us >> 4;
			else us = us & 0xF;

			switch (TargetVolume3)
			{
			default:
			case 0:
				us = 0;
				break;
			case 1:
				break;
			case 2:
				us >>= 1;
				break;
			case 3:
				us >>= 2;
				break;
			}

			int8_t ss = reinterpret_cast<int8_t&>(us);

			s += ss;
			//s += ss - 8;

			if (CurrentCycleIndex3 == 0)
			{
				Pattern3Index++;
				if (Pattern3Index == 32) Pattern3Index = 0;
			}
		}
		else
		{
			Pattern3Index = 0;
			CurrentCycleIndex3 = 0;
		}
#endif

		//stream[i++] = int8_t(s / 3);
		stream[i++] = int8_t(s);
	}
}

SdlSound::SdlSound()
	:deviceId_(0)
{
	for (int i = 0; i < SDL_GetNumAudioDevices(0); i++) {
		const char *name = SDL_GetAudioDeviceName(i, 0);
		printf("Audio %d: '%s'\n", i, name);
	}

	memset(Pattern3, 0, 16);


	SDL_AudioSpec want = {}, have = {};

	want.freq = SampleRate;
	want.format = AUDIO_S8; // AUDIO_U8
	want.channels = 1;
	want.samples = SampleSize;
	want.callback = SDL_AudioCallback;

	deviceId_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
	if (deviceId_ <= 0) {
		printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
		assert(0);
	}

	SDL_PauseAudioDevice(deviceId_, 0); // unpause
}

SdlSound::~SdlSound()
{
	if (deviceId_ > 0)
	{
		SDL_CloseAudioDevice(deviceId_);
		deviceId_ = 0;
	}
}

void SdlSound::Tick(int consumedTicks)
{

}

void SdlSound::SetFrequency1(int freq)
{
	if (freq < 10) freq = 10;
	if (freq > SampleRate / 2) freq = SampleRate / 2; // ?

	TargetFreq1 = freq;
}

void SdlSound::SetVolume1(int volume)
{
	if (volume < 0) volume = 0;
	if (volume > 15) volume = 15;

	TargetVolume1 = volume;
}

void SdlSound::SetFrequency2(int freq)
{
	if (freq < 10) freq = 10;
	if (freq > SampleRate / 2) freq = SampleRate / 2; // ?

	TargetFreq2 = freq;
}

void SdlSound::SetVolume2(int volume)
{
	if (volume < 0) volume = 0;
	if (volume > 15) volume = 15;

	TargetVolume2 = volume;
}

void SdlSound::SetFrequency3(int freq)
{
	if (freq < 10) freq = 10;
	if (freq > SampleRate / 2) freq = SampleRate / 2; // ?

	TargetFreq3 = freq;
}

void SdlSound::SetVolume3(int volume)
{
	if (volume < 0) volume = 0;
	if (volume > 15) volume = 15;

	TargetVolume3 = volume;
}

void SdlSound::SetPattern3(uint8_t *pattern)
{
	memcpy(Pattern3, pattern, 16);
}

void SdlSound::SetPlayback3(bool playback)
{
	Playback3 = playback;
}

}
