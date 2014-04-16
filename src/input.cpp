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
		
		for (int i = 0; i < n_bombers; i++)
		{
			if      (event.key.keysym.sym == config.player_conf[i].key_action1)
				bombers[i].action1 = 1;
			else if (event.key.keysym.sym == config.player_conf[i].key_action2)
				bombers[i].action2 = 1;
			
			else if (event.key.keysym.sym == config.player_conf[i].key_up)
				bombers[i].move_up = 1;
			else if (event.key.keysym.sym == config.player_conf[i].key_down)
				bombers[i].move_down = 1;
			else if (event.key.keysym.sym == config.player_conf[i].key_left)
				bombers[i].move_left = 1;
			else if (event.key.keysym.sym == config.player_conf[i].key_right)
				bombers[i].move_right = 1;
		}
		
		switch (event.key.keysym.sym) {
		case SDLK_ESCAPE: config.quit = 1; break;
		
		case SDLK_4: break;
		
		case SDLK_s: break;
		
		default: break;
		}
		break;
	
	case SDL_KEYUP:
		for (int i = 0; i < n_bombers; i++)
		{
			if      (event.key.keysym.sym == config.player_conf[i].key_action1)
				bombers[i].action1 = 0;
			else if (event.key.keysym.sym == config.player_conf[i].key_action2)
				bombers[i].action2 = 0;
			
			else if (event.key.keysym.sym == config.player_conf[i].key_up)
				bombers[i].move_up = 0;
			else if (event.key.keysym.sym == config.player_conf[i].key_down)
				bombers[i].move_down = 0;
			else if (event.key.keysym.sym == config.player_conf[i].key_left)
				bombers[i].move_left = 0;
			else if (event.key.keysym.sym == config.player_conf[i].key_right)
				bombers[i].move_right = 0;
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

int get_input()
{
	SDL_Event event;
	
	while (SDL_PollEvent(&event))
		dogadjaji.push(Dog(SDL_GetTicks(), event));
	
	int time = SDL_GetTicks();
	while (!dogadjaji.empty() && time - dogadjaji.front().first > 1) {
		event = dogadjaji.front().second; dogadjaji.pop();
		do_event(event);
	}
	
	return 0;
}

