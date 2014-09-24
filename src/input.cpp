/**
  @file    input.c
  @author  ivansandrk
  @date    Aug 2011
  @version 0.1
  @brief   Handling input
*/


#include "SDL.h"

#include "config.h"
#include "bomber.h"
// level.h includes config.h itself
#include "level.h"

#include <queue>
#include <utility>
typedef std::pair<int, SDL_Event> Dog;
std::queue<Dog> dogadjaji;

static SDL_Joystick* joystick;

static inline void do_event(SDL_Event event)
{
	static int mode_brick = 0;
	
	switch (event.type) {
	case SDL_QUIT: config.quit = 1; break;
	
	case SDL_KEYDOWN:
		if (event.key.keysym.sym == config.key_pause_time)
			config.pause_time ^= 1;
		if (event.key.keysym.sym == config.key_bullet_time)
			config.bullet_time ^= 1;
		if (event.key.keysym.sym == config.key_draw_grid)
			config.draw_grid ^= 1;
		if (event.key.keysym.sym == config.key_show_fps)
			config.show_fps ^= 1;
		
		switch (event.key.keysym.sym) {
		case SDLK_ESCAPE: config.quit = 1; break;
		case SDLK_4: break;
		case SDLK_s: break;
		default: break;
		}
	
	case SDL_KEYUP:
		for (int i = 0; i < n_bombers; i++)
		{
			int is_pressed = (event.type == SDL_KEYDOWN);
			if      (event.key.keysym.sym == config.player_conf[i].key_action1)
				bombers[i].action1 = is_pressed;
			else if (event.key.keysym.sym == config.player_conf[i].key_action2)
				bombers[i].action2 = is_pressed;
			
			else if (event.key.keysym.sym == config.player_conf[i].key_up)
				bombers[i].move_up = is_pressed;
			else if (event.key.keysym.sym == config.player_conf[i].key_down)
				bombers[i].move_down = is_pressed;
			else if (event.key.keysym.sym == config.player_conf[i].key_left)
				bombers[i].move_left = is_pressed;
			else if (event.key.keysym.sym == config.player_conf[i].key_right)
				bombers[i].move_right = is_pressed;
		}
		break;
	
	// TODO: hackity-hack to add/remove bricks
	case SDL_MOUSEMOTION:
	case SDL_MOUSEBUTTONDOWN:
	{
		int row = (config.height - event.button.y - LOWER_LEFT_CORNER_Y) / TILE_HEIGHT;
		int col = (event.button.x - LOWER_LEFT_CORNER_X) / TILE_WIDTH;
		if (! (row >= 0 && row < LEVEL_ROWS && col >= 0 && col < LEVEL_COLS))
			break;
		
		if (event.type == SDL_MOUSEMOTION)
		{
			if (board[row][col].type == LEVEL_TILE_EMPTY && mode_brick == 1)
			{
				board[row][col].type = LEVEL_TILE_BRICK;
			}
			else if (board[row][col].type == LEVEL_TILE_BRICK && mode_brick == 2)
			{
				board[row][col].type = LEVEL_TILE_EMPTY;
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (board[row][col].type == LEVEL_TILE_EMPTY)
			{
				mode_brick = 1;
				board[row][col].type = LEVEL_TILE_BRICK;
			}
			else if (board[row][col].type == LEVEL_TILE_BRICK)
			{
				mode_brick = 2;
				board[row][col].type = LEVEL_TILE_EMPTY;
			}
		}
		break;
	}
	
	case SDL_MOUSEBUTTONUP: mode_brick = 0; break;
	
	/* if it needs redrawing swapping buffers is usually enough */
	case SDL_VIDEOEXPOSE: /*SDL_GL_SwapBuffers();*/ break;
	default: break;
	}
	
	// TODO: add mouse input for a player?
	
	// TODO: fix input for AI players and network players
	// probably need some flags (normal player, AI player, network player)
	// AI players set the appropriate values in AI code
	// network players set the appropriate values in network code
}

static inline void do_joystick_event(SDL_Event event)
{
	switch (event.type) {
	case SDL_JOYAXISMOTION:
		// left/right
		if (event.jaxis.axis == 0) {
			if (event.jaxis.value > 0) {
				bombers[0].move_right = 1;
			}
			else if (event.jaxis.value < 0) {
				bombers[0].move_left = 1;
			}
			else {
				bombers[0].move_left = bombers[0].move_right = 0;
			}
		}
		// up/down
		else if (event.jaxis.axis == 1) {
			if (event.jaxis.value > 0) {
				bombers[0].move_down = 1;
			}
			else if (event.jaxis.value < 0) {
				bombers[0].move_up = 1;
			}
			else {
				bombers[0].move_up = bombers[0].move_down = 0;
			}
		}
		break;
	case SDL_JOYHATMOTION:
		bombers[0].move_up    = (event.jhat.value >> 0) & 1;
		bombers[0].move_right = (event.jhat.value >> 1) & 1;
		bombers[0].move_down  = (event.jhat.value >> 2) & 1;
		bombers[0].move_left  = (event.jhat.value >> 3) & 1;
		break;
	case SDL_JOYBUTTONDOWN:
	case SDL_JOYBUTTONUP:
		if (event.jbutton.button == 3) {
			bombers[0].action1 = event.type == SDL_JOYBUTTONDOWN;
		}
		if (event.jbutton.button == 2) {
			bombers[0].action2 = event.type == SDL_JOYBUTTONDOWN;
		}
		break;
	}
}

int get_input()
{
	SDL_Event event;
	
	while (SDL_PollEvent(&event))
		dogadjaji.push(Dog(SDL_GetTicks(), event));
	
	int time = SDL_GetTicks();
	while (!dogadjaji.empty() && time - dogadjaji.front().first > 1) {
		event = dogadjaji.front().second; dogadjaji.pop();
		do_event(event);
		do_joystick_event(event);
	}
	
	return 0;
}

int input_init()
{
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) < 0) {
		fprintf(stderr, "Error initialising SDL Joystick subsystem: %s\n", SDL_GetError());
		return -1;
	}
	
	if (SDL_NumJoysticks() >= 1) {
		joystick = SDL_JoystickOpen(0);
		if (joystick == NULL) {
			fprintf(stderr, "Unable to open joystick: %s\n", SDL_GetError());
			return -1;
		}
		fprintf(stderr, "Joystick name: %s\n", SDL_JoystickName(0));
	}
	
	return 0; // success
}

int input_quit()
{
	if (joystick) {
		SDL_JoystickClose(joystick);
		joystick = NULL;
	}
	
	return 0;
}
