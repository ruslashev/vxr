#include "pixeldrawer.hpp"
#include "game.hpp"
#include <fstream>

int main()
{
	PixelDrawer drawer;

	Init();

	SDL_Event event;
	bool running = true;
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

		auto curtime = []() -> double { return clock() / (double)CLOCKS_PER_SEC; };

		Update(curtime() - lastUpdate);
		lastUpdate = curtime();

		DrawFrame(&drawer);

		drawer.Update();
	}

	Cleanup();

	return 0;
}

