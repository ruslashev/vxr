#include "pixeldrawer.hpp"
#include <fstream>

int main()
{
	PixelDrawer drawer;
	SDL_Event event;
	bool running = true;

	drawer.WritePixel(0, 0, 0xFFFF0000);
	drawer.WritePixel(1, 0, 0xFF00FF00);
	drawer.WritePixel(0, 1, 0xFF0000FF);

	while (running) {
		while (SDL_PollEvent(&event) != 0)
			if (event.type == SDL_QUIT)
				running = false;

		drawer.Update();
	}

	return 0;
}

