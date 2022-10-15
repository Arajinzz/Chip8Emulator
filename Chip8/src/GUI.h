#pragma once

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include "Chip8.h"
#include <SDL.h>
#include <SDL_opengl.h>

class GUI
{

private:
	Chip8* chip8;

public:
	GUI(Chip8* chip8);
	~GUI();

	void Update();

	SDL_Window* window;
	SDL_WindowFlags window_flags;
	SDL_GLContext gl_context;
	ImGuiIO* m_io;

};

