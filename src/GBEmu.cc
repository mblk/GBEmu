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
	printf("GBEmu\n");

	std::string romFileName;

	if (argc > 1)
	{
		romFileName = argv[1];
	}
	else
	{
		//romFileName = "C:\\Users\\Marcel\\Desktop\\GameBoy\\roms\\Final Fantasy Adventure (USA).gb";
		romFileName = "C:\\Users\\Marcel\\Desktop\\GameBoy\\roms\\Super Mario Land (JUE) (V1.1) [!].gb";
		//romFileName = "C:\\Users\\Marcel\\Desktop\\GameBoy\\roms\\Tetris (World).gb";
		//romFileName = "C:\\Users\\Marcel\\Desktop\\GameBoy\\roms\\testroms\\cpu_instrs\\cpu_instrs.gb";
	}

	printf("Rom: '%s'\n", romFileName.c_str());



	SDL_Event event;
	SDL_Renderer *renderer;
	SDL_Window *window;
	int i;

	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(256, 256, 0, &window, &renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	for (i = 0; i < 256; ++i)
		SDL_RenderDrawPoint(renderer, i, i);
	SDL_RenderPresent(renderer);




	auto sdlDisplayBitmap = new GBEmu::SdlDisplayBitmap(renderer);

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
	auto display = new GBEmu::Emulator::Display(*io, *pic, *vram, *oam, *sdlDisplayBitmap);
	auto serial = new GBEmu::Emulator::Serial(*log, *io);
	auto dma = new GBEmu::Emulator::Dma(*io, *memory);
	auto timer = new GBEmu::Emulator::Timer(*log, *io, *pic);
	auto cpu = new GBEmu::Emulator::Cpu(*log, *memory, *io, *pic);




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
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();


	printf("Press any key ...\n");
	getchar();
    return 0;
}

