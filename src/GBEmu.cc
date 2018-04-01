#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>

#include "sdldisplaybitmap.hh"

#include "emulator/log.hh"
#include "emulator/memory.hh"
#include "emulator/rom.hh"
#include "emulator/ram.hh"
#include "emulator/io.hh"
#include "emulator/display.hh"
#include "emulator/pic.hh"
#include "emulator/serial.hh"
#include "emulator/dma.hh"
#include "emulator/timer.hh"
#include "emulator/cpu.hh"

#include <SDL.h>
#undef main

int main(int argc, char **argv)
{
	int ret;
	bool showDebugWindow = true;

	// Determine which ROM file should be used.
	std::string romFileName;
	if (argc > 1)
	{
		romFileName = argv[1];
	}
	else
	{
		//romFileName = "roms\\Final Fantasy Adventure (USA).gb";
		romFileName = "roms\\Super Mario Land (JUE) (V1.1) [!].gb";
		//romFileName = "roms\\Tetris (World).gb";
	}

	// Initialize SDL.
	ret = SDL_Init(SDL_INIT_VIDEO);
	assert(!ret);

	// Get physical screen size.
	SDL_DisplayMode DM;
	ret = SDL_GetCurrentDisplayMode(0, &DM);
	assert(!ret);
	const int screenWidth = DM.w;
	const int screenHeight = DM.h;

	// Get Gameboy display size and determine the window size to be used.
	int originalDisplayWidth, originalDisplayHeight;
	GBEmu::Emulator::Display::GetSize(&originalDisplayWidth, &originalDisplayHeight);
	assert(originalDisplayWidth > 0);
	assert(originalDisplayHeight > 0);
	const int displayZoomFactor = 3;
	const int displayWidth = originalDisplayWidth * displayZoomFactor;
	const int displayHeight = originalDisplayHeight * displayZoomFactor;

	// Create SDL-windows.
	SDL_Event event;
	SDL_Renderer *debugRenderer = nullptr, *displayRenderer = nullptr;
	SDL_Window *debugWindow = nullptr, *displayWindow = nullptr;

	SDL_CreateWindowAndRenderer(displayWidth, displayHeight, 0, &displayWindow, &displayRenderer);
	SDL_SetWindowPosition(displayWindow, screenWidth / 2 - displayWidth / 2, screenHeight / 2 - displayHeight / 2);

	if (showDebugWindow)
	{
		SDL_CreateWindowAndRenderer(256, 256, 0, &debugWindow, &debugRenderer);
		int dwx, dwy;
		SDL_GetWindowPosition(displayWindow, &dwx, &dwy);
		SDL_SetWindowPosition(debugWindow, dwx - 300, dwy);
	}

	// Initialize Emulator.
	auto sdlDebugBitmap = new GBEmu::SdlDisplayBitmap(debugRenderer, 256, 256);
	auto sdlDisplayBitmap = new GBEmu::SdlDisplayBitmap(displayRenderer, 160, 144);

	auto log = new GBEmu::Emulator::Log("log.txt");
	auto rom = new GBEmu::Emulator::Rom(*log);
	rom->Load(romFileName);
	auto ram = new GBEmu::Emulator::Ram();
	auto vram = new GBEmu::Emulator::Ram();
	auto extram = new GBEmu::Emulator::Ram();
	auto oam = new GBEmu::Emulator::SpriteAttributeTable();
	auto io = new GBEmu::Emulator::IO(*log);
	auto memory = new GBEmu::Emulator::Memory(*log);
	memory->Register(rom, 0x0000);
	memory->Register(vram, 0x8000);
	memory->Register(extram, 0xA000);
	memory->Register(ram, 0xC000);
	memory->Register(oam, 0xFE00);
	memory->Register(io, 0xFF00);
	auto pic = new GBEmu::Emulator::Pic(*log, *io);
	auto display = new GBEmu::Emulator::Display(*io, *pic, *vram, *oam, *sdlDebugBitmap, *sdlDisplayBitmap);
	auto serial = new GBEmu::Emulator::Serial(*log, *io);
	auto dma = new GBEmu::Emulator::Dma(*io, *memory);
	auto timer = new GBEmu::Emulator::Timer(*log, *io, *pic);
	auto cpu = new GBEmu::Emulator::Cpu(*log, *memory, *io, *pic);

	// Main loop.
	bool running = true;
	bool left = false, right = false, up = false, down = false, a = false, b = false, start = false, select = false;
	uint32_t totalInstructions = 0;
	auto startTime = std::chrono::high_resolution_clock::now();

	while (running)
	{
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
		double dt = double(ms) * 0.001;

		if (dt > 1.0)
		{
			double ips = double(totalInstructions) / dt;
			printf("IPS: %.1lf\n", ips);
			startTime = std::chrono::high_resolution_clock::now();
			totalInstructions = 0;
		}

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT: running = false; break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE: running = false; break;

				case SDLK_a: left = true; break;
				case SDLK_d: right = true; break;
				case SDLK_w: up = true; break;
				case SDLK_s: down = true; break;
				case SDLK_j: a = true; break;
				case SDLK_k: b = true; break;
				case SDLK_q: start = true; break;
				case SDLK_e: select = true; break;
				}
				break;
				
			case SDL_KEYUP:
				switch (event.key.keysym.sym)
				{
				case SDLK_a: left = false; break;
				case SDLK_d: right = false; break;
				case SDLK_w: up = false; break;
				case SDLK_s: down = false; break;
				case SDLK_j: a = false; break;
				case SDLK_k: b = false; break;
				case SDLK_q: start = false; break;
				case SDLK_e: select = false; break;
				}
				break;
			}
		}

		uint32_t consumedTicks = 0;

		for (int i = 0; i < 128; i++)
		{
			consumedTicks += cpu->Tick();
			totalInstructions++;
		}

		io->SetInput(left, right, up, down, a, b, start, select);
		display->Tick(consumedTicks);
		timer->Tick(consumedTicks);
	}
	
	// Shut down.
	if (displayRenderer) SDL_DestroyRenderer(displayRenderer);
	if (displayWindow) SDL_DestroyWindow(displayWindow);

	if (showDebugWindow)
	{
		if (debugRenderer) SDL_DestroyRenderer(debugRenderer);
		if (debugWindow) SDL_DestroyWindow(debugWindow);
	}

	SDL_Quit();

	// Done.
    return 0;
}
