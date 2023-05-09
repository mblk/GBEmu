#include "emulator/emulator.hh"
#include "sdldisplaybitmap.hh"
#include "sdlsound.hh"
#include "touchui.hh"

#include <cstdio>
#include <cstdlib>
#include <cassert>

#include <string>
#include <memory>
#include <thread>
#include <chrono>

#include <SDL.h>
#undef main

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

GBEmu::Emulator::KeypadKeys keys = {};
std::unique_ptr<GBEmu::Emulator::Emulator> emulator = {};
std::unique_ptr<GBEmu::TouchUi> touchUi = {};

bool running = true;

std::chrono::high_resolution_clock::time_point mainPrevTime_;
int mainCalls_ = 0;

void mainloop(void *arg)
{
	// Statistics.
	const auto now = std::chrono::high_resolution_clock::now();
	const auto dt = std::chrono::duration<double>(now - mainPrevTime_).count();
	mainCalls_++;
	if (dt > 2.5)
	{
		printf("mainloop %.1lf Hz\n", static_cast<double>(mainCalls_) / dt);
		mainPrevTime_ = now;
		mainCalls_ = 0;
	}

	// Helper function that translates sdl keys to emulator keys.
	auto TranslateKeyCode = [](SDL_Keycode keyCode) -> int {
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
			break;
		}

		}

		touchUi->ProcessEvent(event);
	}

	// Render ui first as it clears the entire window.
	touchUi->Render();

	std::array<bool, 8> touchUiKeys = {};
	touchUi->GetKeyStates(touchUiKeys);

	std::array<bool, 8> combinedKeys = {};
	for(int i=0; i<8; i++)
		combinedKeys[i] = keys[i] || touchUiKeys[i];

	// Update emulator.
	emulator->Tick(combinedKeys);
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
	const int displayZoomFactor = 4;

	// Determine which ROM file should be used.
	std::string romFileName;
	if (argc > 1)
	{
		romFileName = argv[1];
	}
	else
	{
#ifdef __EMSCRIPTEN__
		//romFileName = "roms/tetris.gb";
		romFileName = "roms/mario.gb";
#else
		//romFileName = "roms/tetris.gb";
		romFileName = "roms/mario.gb";
#endif
	}

	std::string fontFileName;
#ifdef __EMSCRIPTEN__
		fontFileName = "assets/DejaVuSans.ttf";
#else
		fontFileName = "../assets/DejaVuSans.ttf";
#endif

	// Initialize SDL.
	int ret;
	ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	assert(!ret);

	// xxx
	int numTouchDevices = SDL_GetNumTouchDevices();
	printf("numTouchDevices: %d\n", numTouchDevices);

	// Determine which display to use based on the mouse position.
#ifdef __EMSCRIPTEN__
	int displayIndexForWindow = 0;
#else
	SDL_Point globalMousePos = {};
	SDL_GetGlobalMouseState(&globalMousePos.x, &globalMousePos.y);
	int displayIndexContainingMouse = SDL_GetPointDisplayIndex(&globalMousePos);
	int displayIndexForWindow = displayIndexContainingMouse >= 0
		? displayIndexContainingMouse
	 	: 0;
#endif
	SDL_Rect displayBounds = {};
	SDL_GetDisplayBounds(displayIndexForWindow, &displayBounds);
	printf("display bounds: %d %d %d %d", displayBounds.x, displayBounds.y, displayBounds.w, displayBounds.h);

	// Get Gameboy display size and determine the window size to be used.
	int originalDisplayWidth, originalDisplayHeight;
	GBEmu::Emulator::Display::GetSize(&originalDisplayWidth, &originalDisplayHeight);
	assert(originalDisplayWidth > 0);
	assert(originalDisplayHeight > 0);
	const int displayWidth = originalDisplayWidth * displayZoomFactor;
	const int displayHeight = originalDisplayHeight * displayZoomFactor * 2; // TODO 16:9 / 9:16 ?
	printf("display size: %d %d\n", displayWidth, displayHeight);

	// Create SDL-windows.
	SDL_Renderer *debugRenderer = nullptr, *displayRenderer = nullptr;
	SDL_Window *debugWindow = nullptr, *displayWindow = nullptr;
	SDL_CreateWindowAndRenderer(displayWidth, displayHeight, 0, &displayWindow, &displayRenderer);
	SDL_SetWindowPosition(displayWindow,
		displayBounds.x + displayBounds.w / 2 - displayWidth / 2,
		displayBounds.y + displayBounds.h / 2 - displayHeight / 2);
	SDL_SetWindowTitle(displayWindow, "GBEmu");

	if (showDebugWindow)
	{
		SDL_CreateWindowAndRenderer(256, 256, 0, &debugWindow, &debugRenderer);
		int dwx, dwy;
		SDL_GetWindowPosition(displayWindow, &dwx, &dwy);
		SDL_SetWindowPosition(debugWindow, dwx - 300, dwy);
	}

	// Initialize Emulator.
	const SDL_Rect displayRect = {
		.x = 0,
		.y = 0,
		.w = displayWidth,
		.h = displayHeight
	};
	const SDL_Rect displayPresentRect = {
		.x = 0,
		.y = 0,
		.w = displayWidth,
		.h = displayHeight / 2
	};
	const SDL_Rect uiRect = {
		.x = 0,
		.y = displayHeight / 2,
		.w = displayWidth,
		.h = displayHeight / 2
	};
	const SDL_Rect debugPresentRect = {
		.x = 0,
		.y = 0,
		.w = 256,
		.h = 256
	};

	auto sdlDebugBitmap = showDebugWindow ? std::make_unique<GBEmu::SdlDisplayBitmap>(debugRenderer, debugPresentRect, 256, 256) : nullptr;
	auto sdlDisplayBitmap = std::make_unique<GBEmu::SdlDisplayBitmap>(displayRenderer, displayPresentRect, originalDisplayWidth, originalDisplayHeight);
	auto sdlSound = std::make_unique<GBEmu::SdlSound>();
	emulator = std::make_unique<GBEmu::Emulator::Emulator>("log.txt", romFileName, sdlDebugBitmap.get(), *sdlDisplayBitmap, *sdlSound);

	touchUi = std::make_unique<GBEmu::TouchUi>(displayRenderer, displayRect, uiRect);
	touchUi->Initialize(fontFileName);

	// Main loop.
#ifdef __EMSCRIPTEN__
	int targetFps = -1;
	int simulateInfiniteLoop = 1;
	emscripten_set_main_loop_arg(mainloop, nullptr, targetFps, simulateInfiniteLoop);
#else
	while (running)
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
