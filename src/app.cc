#include "app.hh"
#include "sdlhelper.hh"
#include "startui.hh"
#include "touchui.hh"
#include "emulator/emulator.hh"
#include "sdldisplaybitmap.hh"
#include "sdlsound.hh"
#include "romstore.hh"

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

namespace GBEmu
{

App::App()
    :running_(true),
    state_(AS_INVALID),
    commands_({})
{
    commands_.push_back(Command {
        .type_ = CMD_START_MENU,
    });

    romStore_ = std::make_unique<RomStore>();
    romStore_->AddFromFile("roms/tetris.gb");
    romStore_->AddFromFile("roms/mario.gb");
}

App::~App()
{
}

int App::Initialize()
{
    sdlHelper_ = std::make_unique<SdlHelper>();
    if (sdlHelper_->Initialize()) {
        printf("SdlHelper::Initialize() failed\n");
        return -1;
    }

    return 0;
}

int App::Shutdown()
{
    sdlHelper_->Shutdown();

    return 0;
}

void App::MainloopWrapper(void *arg)
{
    App *app = reinterpret_cast<App*>(arg);
    app->Mainloop();
}

void App::Run()
{
#ifdef __EMSCRIPTEN__
	int targetFps = -1;
	int simulateInfiniteLoop = 1;
	emscripten_set_main_loop_arg(MainloopWrapper, this, targetFps, simulateInfiniteLoop);
#else
	while (running_)
		Mainloop();
#endif
}

void App::Mainloop()
{
	const auto now = std::chrono::high_resolution_clock::now();
    if (mainFirstCall_) {
        mainFirstCall_ = false;
        mainPrevTime_ = now;
    }
	const double dt = std::chrono::duration<double>(now - mainPrevTime_).count();
    mainPrevTime_ = now;

	mainCalls_++;
    mainTime_ += dt;

    if (mainTime_ > 2.5)
    {
        double fps = static_cast<double>(mainCalls_) / mainTime_;
        mainCalls_ = 0;
        mainTime_ = 0;
        printf("mainloop %0.1f FPS\n", fps);
    }

    // Process commands
    if (commands_.size() > 0)
    {
        for(const Command& command : commands_)
        {
            switch(command.type_)
            {
                case CMD_START_MENU:
                    printf("CMD_START_MENU\n");
                    StopEmulator();
                    StartMenu();
                    break;

                case CMD_START_EMULATOR:
                    printf("CMD_START_EMULATOR\n");
                    StopMenu();
                    StartEmulator(command.romId_);
                    break;
            }
        }
        commands_.clear();
    }

	// Process SDL events.
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			running_ = false;
			break;

		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				running_ = false;
			break;
		}

        switch(state_)
        {
        case AS_MENU:
            if (startUi_) startUi_->ProcessEvent(event);
            break;

        case AS_EMULATOR:
            if (touchUi_) touchUi_->ProcessEvent(event);
            break;

        default: break;
        }
	}

    sdlHelper_->Clear();

    switch (state_)
    {
    case AS_MENU:
        if (startUi_) startUi_->Render();
        break;

    case AS_EMULATOR:
        if (touchUi_ && emulator_) {
            std::array<bool, 8> keys = {};
            touchUi_->GetKeyStates(keys);
            emulator_->Tick(dt, keys);

            sdlHelper_->GetDisplayBitmap().Render();
            touchUi_->Render();
        }
        break;

    default: break;
    }

    sdlHelper_->Present();
}

void App::StartEmulator(int romId)
{
    const RomData* rom = romStore_->GetRom(romId);
    if (!rom) {
        printf("Rom %d not found\n", romId);
        return;
    }

    emulator_ = new Emulator::Emulator("log.txt", rom->size_, rom->data_, NULL, 
        sdlHelper_->GetDisplayBitmap(), sdlHelper_->GetSound());

    const SDL_Rect windowRect = sdlHelper_->GetWindowRect();

    const SDL_Rect uiRect = {
        .x = 0,
        .y = windowRect.h / 2,
        .w = windowRect.w,
        .h = windowRect.h / 2
    };

    touchUi_ = new TouchUi(*sdlHelper_, windowRect, uiRect);
    touchUi_->RegisterExit([&]() {
        commands_.push_back(Command {
            .type_ = CMD_START_MENU,
        });
    });

    state_ = AS_EMULATOR;
}

void App::StopEmulator()
{
    if (touchUi_) {
        delete touchUi_;
        touchUi_ = nullptr;
    }
    if (emulator_) {
        delete emulator_;
        emulator_ = nullptr;
    }
}

void App::StartMenu()
{
    startUi_ = new StartUi(*sdlHelper_, *romStore_);
    startUi_->RegisterLoad([&](int romId) {
        commands_.push_back(Command {
            .type_ = CMD_START_EMULATOR,
            .romId_ = romId,
        });
    });

    state_ = AS_MENU;
}

void App::StopMenu()
{
    if (startUi_)
    {
        delete startUi_;
        startUi_ = nullptr;
    }
}

}
