#include "emulator/emulator.hh"
#include "sdldisplaybitmap.hh"
#include "sdlsound.hh"

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <string>
#include <memory>
#include <thread>

#include <SDL.h>
#undef main

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

GBEmu::Emulator::KeypadKeys keys = {};
std::unique_ptr<GBEmu::Emulator::Emulator> emulator = {};
bool running = true;

// Helper function that translates sdl keys to emulator keys.
auto TranslateKeyCode = [](SDL_Keycode keyCode) -> int
{
	switch (keyCode)
	{
	case SDLK_a: return GBEmu::Emulator::Keypad::Left;
	case SDLK_d: return GBEmu::Emulator::Keypad::Right;
	case SDLK_w: return GBEmu::Emulator::Keypad::Up;
	case SDLK_s: return GBEmu::Emulator::Keypad::Down;
	case SDLK_j: return GBEmu::Emulator::Keypad::A;
	case SDLK_k: return GBEmu::Emulator::Keypad::B;
	case SDLK_q: return GBEmu::Emulator::Keypad::Start;
	case SDLK_e: return GBEmu::Emulator::Keypad::Select;
	default: return -1;
	}
};

void mainloop(void *arg)
{
	//printf("mainloop\n");

	// Process SDL events.
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			running = false;
			break;

		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			auto keyCode = event.key.keysym.sym;
			if (keyCode == SDLK_ESCAPE) {
				running = false;
				break;
			}

			int key = TranslateKeyCode(keyCode);
			if (key >= 0 && key < 8)
			{
				keys[key] = event.type == SDL_KEYDOWN;
			}
		}
		}
	}

	// Update emulator.
	emulator->Tick(keys);
}

// void workerfunc()
// {
// 	while(1)
// 	{
// 		printf("hello from thread\n");
// 		std::this_thread::sleep_for(std::chrono::seconds(1));
// 	}
// }

int main(int argc, char **argv)
{
	printf("main()\n");

	printf("argc: %d\n", argc);
	for(int i=0; i<argc; i++)
		printf("argv[%d] = '%s'\n", i, argv[i]);

	// xxx
	//std::thread worker(workerfunc);
	// xxx

	const bool showDebugWindow = false;
	const int displayZoomFactor = 3;

	// Determine which ROM file should be used.
	std::string romFileName;
	if (argc > 1)
	{
		romFileName = argv[1];
	}
	else
	{
#ifdef __EMSCRIPTEN__
		romFileName = "tetris.gb";
		//romFileName = "mario.gb";
#else
		//romFileName = "roms\\Final Fantasy Adventure (USA).gb";
		//romFileName = "roms\\Super Mario Land (JUE) (V1.1) [!].gb";
		romFileName = "roms\\Tetris (World).gb";
#endif
	}

	// Initialize SDL.
	int ret;
	ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
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
	const int displayWidth = originalDisplayWidth * displayZoomFactor;
	const int displayHeight = originalDisplayHeight * displayZoomFactor;

	// Create SDL-windows.
	SDL_Renderer *debugRenderer = nullptr, *displayRenderer = nullptr;
	SDL_Window *debugWindow = nullptr, *displayWindow = nullptr;

	SDL_CreateWindowAndRenderer(displayWidth, displayHeight, 0, &displayWindow, &displayRenderer);
	SDL_SetWindowPosition(displayWindow, screenWidth / 2 - displayWidth / 2, screenHeight / 2 - displayHeight / 2);
	SDL_SetWindowTitle(displayWindow, "GBEmu");

	if (showDebugWindow)
	{
		SDL_CreateWindowAndRenderer(256, 256, 0, &debugWindow, &debugRenderer);
		int dwx, dwy;
		SDL_GetWindowPosition(displayWindow, &dwx, &dwy);
		SDL_SetWindowPosition(debugWindow, dwx - 300, dwy);
	}

	// Initialize Emulator.
	auto sdlDebugBitmap = showDebugWindow ? std::make_unique<GBEmu::SdlDisplayBitmap>(debugRenderer, 256, 256) : nullptr;
	auto sdlDisplayBitmap = std::make_unique<GBEmu::SdlDisplayBitmap>(displayRenderer, originalDisplayWidth, originalDisplayHeight);
	auto sdlSound = std::make_unique<GBEmu::SdlSound>();
	emulator = std::make_unique<GBEmu::Emulator::Emulator>("log.txt", romFileName, sdlDebugBitmap.get(), *sdlDisplayBitmap, *sdlSound);

	// Main loop.
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(mainloop, nullptr, -1, 1);
#else
	while(running)
	{
		mainloop(nullptr);
	}
#endif
	
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
