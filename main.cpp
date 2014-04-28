#include <SDL2/SDL.h>
#include <fstream>
#include <memory>

const int WindowWidth = 800, WindowHeight = 600;

class PixelDrawer
{
public:
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	Uint32 pixelData[WindowWidth*WindowHeight];

	PixelDrawer()
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			printf("Failed to initialize SDL\nSDL Error: %s\n", SDL_GetError());
			throw;
		}

		// Create window
		window = SDL_CreateWindow("vxr",
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
				SDL_PIXELFORMAT_ARGB8888, // TODO
				SDL_TEXTUREACCESS_STREAMING, WindowWidth, WindowHeight);
	}
	~PixelDrawer() {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void Update() {
		SDL_UpdateTexture(texture, NULL, pixelData, WindowWidth*sizeof(Uint32));
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	void WritePixel(int x, int y, Uint32 color) {
		pixelData[y*WindowWidth + x] = color;
	}
};

int main()
{
	PixelDrawer drawer;
	SDL_Event event;
	bool running = true;

	drawer.WritePixel(0, 0, 0xFFFF0000);
	drawer.WritePixel(1, 0, 0xFF00FF00);

	while (running) {
		while (SDL_PollEvent(&event) != 0)
			if (event.type == SDL_QUIT)
				running = false;

		drawer.Update();
	}

	return 0;
}

