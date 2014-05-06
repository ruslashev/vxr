#include "pixeldrawer.hpp"
#include "game.hpp"
#include <fstream>

int main()
{
	PixelDrawer drawer;

	Init();

	auto curtime = []() -> double { return clock() / (double)CLOCKS_PER_SEC; };
	double beginTime = curtime();
	DrawFrame(&drawer);
	drawer.Update();
	double endTime = curtime();
	printf("Finished in %fms\n", (endTime-beginTime)*1000);

	SDL_Event event;
	bool running = true;
	while (running)
		while (SDL_PollEvent(&event) != 0)
			if (event.type == SDL_QUIT)
				running = false;

#if 0
	double lastUpdate = 0.0;
	while (running) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT)
				running = false;
			else if (event.type == SDL_KEYDOWN)
				handleInput(SDL_GetKeyboardState(NULL), true);
			else if (event.type == SDL_KEYUP)
				handleInput(SDL_GetKeyboardState(NULL), false);
		}

		Update(curtime() - lastUpdate);
		lastUpdate = curtime();

		DrawFrame(&drawer);

		drawer.Update();
	}
#endif

	Cleanup();

	return 0;
}

