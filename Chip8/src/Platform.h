#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>

class Platform
{

public:
	Platform(const char* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
	~Platform();

	void Update(void const* buffer, int pitch);
	bool ProcessInput(uint8_t* keys);
private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;

};

