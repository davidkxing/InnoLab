#pragma once

#include "../game.h"

class MainMenu {
public:
	float x;
	float y;
	float w;
	float h;
	MainMenu(float tempX, float tempY, float tempW, float tempH);
	void displayButtons(uint8_t* frame_buf, int screen_width, int screen_height);
	void displaySelection(uint8_t* frame_buf, int screen_width, int screen_height);
	bool checkState(bool selection);
	void close_game();
};