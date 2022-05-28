//every variable and function in here needs to be defined in game.cpp
#pragma once

#include "core.h"
#include "pong.h"
#include "Logic/MainMenu.h"
#include "Logic/Timer.h"

extern bool key_up;
extern bool key_down;
extern bool key_left;
extern bool key_right;
extern bool key_start;


void game_init();
void game_deinit();
void game_reset();
void game_print(uint8_t* frame_buf);
void game_run();
void menu_print(uint8_t* frame_buf);
int inGameCheck();


