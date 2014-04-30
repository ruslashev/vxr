#include "pixeldrawer.hpp"
#include "game.hpp"
#include <fstream>

int main()
{
	PixelDrawer drawer;

	drawer.WritePixel(0, 0, 0xFFFF0000);
	drawer.WritePixel(1, 0, 0xFF00FF00);
	drawer.WritePixel(0, 1, 0xFF0000FF);

	SDL_Event event;
	bool running = true;
	double lastUpdate = 0.0;
	while (running) {
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT)
				running = false;

			if (event.type == SDL_KEYDOWN)
				handleInput(SDL_GetKeyboardState(NULL), true);

			if (event.type == SDL_KEYUP)
				handleInput(SDL_GetKeyboardState(NULL), false);
		}

#define CURTIME() (clock() / (float) CLOCKS_PER_SEC)
		Update(CURTIME() - lastUpdate);
		lastUpdate = CURTIME();
		DrawFrame(&drawer);

		drawer.Update();
	}

	return 0;
}

