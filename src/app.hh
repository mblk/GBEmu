#pragma once

#include <memory>
#include <chrono>
#include <list>

namespace GBEmu
{

class SdlHelper;
class StartUi;
class TouchUi;

class RomStore;

namespace Emulator {
    class Emulator;
}

enum AppState
{
    AS_INVALID,
    AS_MENU,
    AS_EMULATOR,
};

enum CommandType
{
    CMD_START_MENU,
    CMD_START_EMULATOR,
};

struct Command
{
    CommandType type_;
    int romId_ = 0;
};

class App
{
public:
    App();
    virtual ~App();

    int Initialize();
    int Shutdown();

    void Run();

private:
    static void MainloopWrapper(void *arg);
    void Mainloop();

    void StartEmulator(int romId);
    void StopEmulator();

    void StartMenu();
    void StopMenu();

private:
    std::unique_ptr<SdlHelper> sdlHelper_;
    std::unique_ptr<RomStore> romStore_;

    bool running_;
    AppState state_;

    bool mainFirstCall_ = true;
    std::chrono::high_resolution_clock::time_point mainPrevTime_;
    int mainCalls_ = 0;
    double mainTime_ = 0;

    std::list<Command> commands_;

    StartUi *startUi_;

    Emulator::Emulator *emulator_;
    TouchUi *touchUi_;

};

}