#include "app.hh"

#include <cstdio>
#include <cstdlib>
#include <cassert>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int main(int argc, char **argv)
{
	GBEmu::App *app = new GBEmu::App();
	
	if (app->Initialize()) {
		printf("App::Initialize() failed\n");
		return -1;
	}

	app->Run();
	app->Shutdown();

    return 0;
}
