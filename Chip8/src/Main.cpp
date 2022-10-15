// http://devernay.free.fr/hacks/chip8/C8TECH10.HTM

#include <iostream>
#include <chrono>
#include <thread>

#include "Chip8.h"
#include "Platform.h"
#include "GUI.h"

int main() {

	int videoScale = 10;

	Chip8 chip8;
	chip8.LoadRom("Soccer.ch8");

	Platform *platform = new Platform("Test", chip8.VIDEO_WIDTH * videoScale, chip8.VIDEO_HEIGHT * videoScale, chip8.VIDEO_WIDTH, chip8.VIDEO_HEIGHT);
	GUI *gui = new GUI(&chip8);

	int videoPitch = sizeof(chip8.video[0]) * chip8.VIDEO_WIDTH;

	bool quit = false;

	auto lastCycleTime = std::chrono::high_resolution_clock::now();

	int iterations = 5000;
	int i = 0;

	while (!quit) {
		
		auto currentTime = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();
		quit = platform->ProcessInput(chip8.keypad);

		if (dt > 1)
		{
			lastCycleTime = currentTime;

			chip8.Cycle();
			platform->Update(chip8.video, videoPitch);
		}

		if (i > iterations) {
			gui->Update();
			i = 0;
		}

		i++;

	}

	return 0;
}