#ifndef PIXELDRAWER_HPP
#define PIXELDRAWER_HPP

#include <SDL2/SDL.h>
#include <memory>

const int WindowWidth = 800, WindowHeight = 600;

class PixelDrawer
{
private:
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	Uint32 pixelData[WindowWidth*WindowHeight];

public:
	PixelDrawer();
	~PixelDrawer();

	void Update();
	void WritePixel(int x, int y, Uint32 color);
};


#endif

