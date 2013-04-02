/*
 * Testing stuff for JDV Bomberman
 *
 */

#include <stdio.h>
#include <unistd.h>
//#include "SDL.h"
//#include "SDL_opengl.h"
#include <time.h>

#include "config.h"
#include "graphics.h"
#include "bomber.h"
#include "input.h"
#include "level.h"
#include "bomb.h"
#include "sound.h"
 

// TODO: add "static xyz time_start" which represents start time
//       so i can calculate the accumulating error of delta time's
//       (save sum of them in another variable)
// first call is the initialising call (delta_time is bogus after first call)
int calc_delta_time()
{
#ifdef TIME_MEASURING_clock_gettime
	static struct timespec last_tp;
	struct timespec tp;
#elif defined TIME_MEASURING_SDL_GetTicks
	static unsigned int last_tick;
	unsigned int tick;
#else
#error define TIME_MEASURING_* in config.h
#endif
	
#ifdef TIME_MEASURING_clock_gettime
	// /usr/include/bits/time.h
	clock_gettime(CLOCK_REALTIME, &tp);
	//clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
	
	delta_time_real =
	delta_time      = (tp.tv_sec - last_tp.tv_sec) +
	                  (tp.tv_nsec - last_tp.tv_nsec) / 1000000000.0f;
	
	last_tp = tp;
#elif defined TIME_MEASURING_SDL_GetTicks
	tick = SDL_GetTicks();
	
	delta_time_real =
	delta_time      = (tick - last_tick) / 1000.0f;
	
	last_tick = tick;
#endif
	
	if (pause_time)
		delta_time = 0;
	else if (bullet_time)
		delta_time *= 0.5f; // TODO: this could be interesting, i can speed up the game :)
	
	return 0;
}


int init()
{
	srand(time(0) ^ getpid());
	
	if (SDL_Init(0) < 0)
	{
		fprintf(stderr, "Error initialising SDL: %s\n", SDL_GetError());
		return -1;
	}
	
	if (config_init() == -1 ||
	    graphics_init(config.width, config.height, config.bits_per_pixel, config.caption) == -1 ||
	    sound_init() == -1 ||
	    level_init() == -1 ||
	    bomber_init() == -1 ||
	    bomb_init() == -1)
	{
		return -1;
	}
	
	// load images/animations - its done up ^ in init's
	
	// init calc_delta_time
	calc_delta_time();
	
	return 0;
}

int clean_up()
{
	// free images / animations
	
	bomb_quit();
	bomber_quit();
	level_quit();
	sound_quit();
	graphics_quit();
	config_quit();
	
	return 0;
}

int draw_all()
{
	//clear_screen();
	drawing_begin();
	
	level_draw_all();
	if (config.draw_grid)
		draw_grid();
	
	bomber_draw_shadows();
	bomb_draw_all();
	bomber_draw_bombers();
	
	// TODO: move this somewhere else?
	char buf[128];
	
	sprintf(buf, "FPS %d", frames_per_second);
	print(570, 460, buf);
	
	drawing_end();
	
	
	return 0;
}

int process()
{
	bomber_process();
	bomb_process();
	
	return 0;
}

int calc_fps()
{
	static int frames = 0;
	static float fps_time = 0.0f;
	
	calc_delta_time();
	
	frames++;
	fps_time += delta_time_real;
	if (fps_time > 1.0f)
	{
		frames_per_second = frames / fps_time;
		fps_time = 0.0f;
		frames = 0;
	}
	
	return 0;
}

int main(int argc, char *argv[])
{
	// to prevent gcc from complaining about unused variables
	(void) argc; (void) argv;
	
	if (init() == -1)
		return 0;
	
	sound_play_loop(SOUND_WIN);
	while (!quit)
	{
		get_input();
		
		// delta_time needs to be set right before calculating moves
		calc_fps();
		process();
		
		draw_all();
		
		//usleep(5 * 1000);
		SDL_Delay(5);
	}
	
	clean_up();
	
	return 0;
}



