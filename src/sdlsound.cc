#include "sdlsound.hh"

#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <cmath>

#include <SDL.h>
#undef main

namespace GBEmu
{

SdlSound::SdlSound()
	:deviceId_(0),
	ticks_(0)
{
	// for (int i = 0; i < SDL_GetNumAudioDevices(0); i++) {
	// 	const char *name = SDL_GetAudioDeviceName(i, 0);
	// 	printf("Audio %d: '%s'\n", i, name);
	// }

	SDL_AudioSpec want = {}, have = {};

	want.freq = SampleRate;
	want.format = AUDIO_S8;
	want.channels = 1;
	want.samples = SampleSize;
	want.callback = SDL_AudioCallback;
	want.userdata = reinterpret_cast<void*>(this);

	deviceId_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
	if (deviceId_ <= 0) {
		printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
		return;
	}

	// printf("Freq: %u\n", have.freq);
	// printf("Format: %u (%u)\n", have.format, AUDIO_S8);
	// printf("Channels: %u\n", have.channels);
	// printf("Samples: %u\n", have.samples);
	// printf("Size: %u\n", have.size);
	// printf("silence: %u\n", have.silence);
	// printf("padding: %u\n", have.padding);

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
	const int targetTicks = 4194304 / SampleRate; // TODO magic number

	ticks_ += consumedTicks;
	while (ticks_ > targetTicks)
	{
		ticks_ -= targetTicks;
		if (ticks_ < 0) ticks_ = 0;

		// Emit audio sample.
		auto sampleData = EmitSample();
		if (sampleBuffer_.RemainingSpace() < 1)
			printf("Audio buffer overrun\n");
		sampleBuffer_.Push(sampleData);
	}
}

int8_t SdlSound::EmitSample()
{
	int8_t sample = 0;
	sample += channel1_.EmitSample();
	sample += channel2_.EmitSample();
	sample += channel3_.EmitSample();
	return sample;
}

void SdlSound::SDL_AudioCallback(void *userdata, uint8_t* stream_, int len)
{
	SdlSound * const sdlSound = reinterpret_cast<SdlSound * const>(userdata);
	assert(sdlSound);
	int8_t * const stream = reinterpret_cast<int8_t*>(stream_);
	assert(stream);

	if(sdlSound->sampleBuffer_.DataSize() < len * 2) {
		//printf("Audio buffer underrun\n");
		memset(stream, 0, len);
		return;
	}

	for(int i=0; i<len; i++)
		stream[i] = sdlSound->sampleBuffer_.Get();
}

}
