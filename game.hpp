#ifndef GAME_HPP
#define GAME_HPP

#include "pixeldrawer.hpp"

#include <SDL2/SDL.h>
#include <cstdint>
#include <cmath>
#include <cstdlib>

void Init();
void Update(double dt);
void handleInput(const uint8_t *states, bool down);
void DrawFrame(PixelDrawer *drawer);
void Cleanup();

#endif

