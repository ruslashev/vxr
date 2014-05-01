#include "pixeldrawer.hpp"

PixelDrawer::PixelDrawer()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Failed to initialize SDL\nSDL Error: %s\n", SDL_GetError());
		throw;
	}

	// Create window
	window = SDL_CreateWindow("Vxr",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			WindowWidth, WindowHeight,
			SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Failed to create Window\nSDL Error: %s\n", SDL_GetError());
		throw;
	}

	// Create a renderer for window
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL) {
		printf("Failed to create Renderer\nSDL Error: %s\n", SDL_GetError());
		throw;
	}

	texture = SDL_CreateTexture(renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, WindowWidth, WindowHeight);
}

void PixelDrawer::Update()
{
	SDL_UpdateTexture(texture, NULL, pixelData, WindowWidth*sizeof(Uint32));
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void PixelDrawer::WritePixel(int x, int y, Uint32 color)
{
	pixelData[y*WindowWidth + x] = color;
}

PixelDrawer::~PixelDrawer()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

