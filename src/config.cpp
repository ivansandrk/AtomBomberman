/**
  @file    config.c
  @author  ivansandrk
  @date    Aug 2011
  @version 0.1
  @brief   Config parameters, some helper functions, loading stuff, initialising and exiting
  
  Configuration parameters are encapsulated inside struct config.
*/

#include "config.h"
#include "graphics.h"
#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

//#define USE_SDL_ZZIP
#include "iniparser.h"



CONFIG config;
float delta_time;
float delta_time_real;
int frames_per_second;

static IniParser* ini;
static IniParser* constants;


int config_init()
{
	int i;
	char buf[1024];
	
	
	ini       = ParseFile(OpenFile(CONFIG_FILE));
	constants = ParseFile(OpenRW(CONSTANTS_FILE));
	
	if (ini == NULL || constants == NULL)
		return -1;
	
	config.graphics_renderer = ini_getconstant("Video", "renderer", AUTO_RENDERER);
	
	config.width = SCREEN_WIDTH;
	config.height = SCREEN_HEIGHT;
	config.bits_per_pixel = SCREEN_BITS_PER_PIXEL;
	
	config.num_players = ini->get_int("Game", "num_players", 0);
	if (config.num_players == 0)
		fprintf(stderr, "Check config.ini, [Game] num_players\n");
	for (i = 0; i < config.num_players; i++)
	{
		sprintf(buf, "Player%d", i);
		config.player_conf[i].key_up      = ini_getkey(buf, "key_up");
		config.player_conf[i].key_down    = ini_getkey(buf, "key_down");
		config.player_conf[i].key_left    = ini_getkey(buf, "key_left");
		config.player_conf[i].key_right   = ini_getkey(buf, "key_right");
		config.player_conf[i].key_action1 = ini_getkey(buf, "key_action1");
		config.player_conf[i].key_action2 = ini_getkey(buf, "key_action2");
		config.player_conf[i].color       = ini_getconstant(buf, "color");
		config.player_conf[i].team        = ini->get_int(buf, "team", 0);
	}
	
	config.level_layout = ini->get_int("Level", "layout", -1);
	assert(config.level_layout != -1);
	config.level_scheme = ini->get_string("Level", "level_scheme");
	assert(config.level_scheme);
	config.starting_player_speed    = ini->get_float("Game", "starting_player_speed",    STARTING_PLAYER_SPEED);
	config.walk_anim_frame_distance = ini->get_float("Game", "walk_anim_frame_distance", WALK_ANIM_FRAME_DISTANCE);
	
	config.draw_grid = ini->get_int("Video", "draw_grid", 0);
	config.show_fps  = ini->get_int("Video", "show_fps" , 0);
	
	config.key_pause_time  = ini_getkey("Game", "key_pause_time");
	config.key_bullet_time = ini_getkey("Game", "key_bullet_time");
	config.key_draw_grid   = ini_getkey("Game", "key_draw_grid");
	config.key_show_fps    = ini_getkey("Game", "key_show_fps");
	
	return 0;
}

int config_quit()
{
	delete ini->get_io();
	delete constants->get_io();
	delete ini;
	delete constants;
	
	return 0;
}


char* SDL_RWReadLine(char *buf, int size, SDL_RWops *rw)
{
	int pos;
	char c;
	
	if (!buf || size <= 0 || !rw)
	{
		return 0;
	}
	
	for (pos = 0; pos < size-1; pos++)
	{
		// on EOF or newline stop reading
		// read the newline, put it into buffer
		if (SDL_RWread(rw, &c, 1, 1) == 0)
		{
			break;
		}
		buf[pos] = c;
		if (c == '\n')
		{
			pos++;
			break;
		}
	}
	buf[pos] = '\0';
	if (pos == 0)
	{
		return 0;
	}
	else
	{
		return buf;
	}
}

char* SDL_RWReadLine_comment(char *buf, int size, SDL_RWops *rw)
{
	char *ret, *tmp;
	
	for (;;)
	{
		ret = SDL_RWReadLine(buf, size, rw);
		
		// EOF
		if (ret == 0)
			return 0;
		
		// empty line or comment (ignore spaces and tabs)
		// skip this line, read next one
		for (tmp = ret; isblank(*tmp); tmp++)
			{}
		if (*tmp == '\n' || *tmp == ';')
			continue;
		
		return ret;
	}
}

char RW_file[4096];
char* RW_readline(SDL_RWops *rw)
{
	static char buf[4096];
	
	if (SDL_RWReadLine_comment(buf, sizeof(buf), rw) == 0)
	{
		fprintf(stderr, "Premature end of %s.\n", RW_file);
		exit(-1);
	}
	else if (strlen(buf) == sizeof(buf)-1)
	{
		fprintf(stderr, "In %s, the following line is too long: [max %ld characters]\n%s\n", RW_file, sizeof(buf)-1, buf);
		exit(-1);
	}
	
	return buf;
}

int RW_scanf(SDL_RWops *rw, const char *format, ...)
{
	va_list args;
	int rc;
	char *buf;
	
	buf = RW_readline(rw);
	
	va_start(args, format);
	rc = vsscanf(buf, format, args);
	va_end(args);
	
	return rc;
}

int RW_file_set(const char *file)
{
	strcpy(RW_file, file);
	
	return 0;
}

int ini_getconstant(const char *section, const char *key, int def_val)
{
	const char *str;
	
	str = ini->get_string(section, key);
	return constants->get_int("Constants", str, def_val);
}

SDLKey ini_getkey(const char *section, const char *key)
{
	return (SDLKey) ini_getconstant(section, key);
}
