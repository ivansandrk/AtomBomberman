/**
  @file    config.c
  @author  johndoevodka
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

CONFIG config;
static dictionary *dict_config_ini;
float delta_time;
float delta_time_real;
int frames_per_second;

// dummy structure for saving ANI's
// -------make sure its only visible in this file - add static qualifier
// -------(had some nasty bug, it probably accessed the other file's variables)
// + now its all in one place (right here in config.c)
ANI *dummy[100];
int ndummy;


int config_init()
{
	int i;
	char buf[1024];
	
	
	dict_config_ini = iniparser_load(CONFIG_FILE);
	
	config.graphics_renderer = ini_getint_symbolic(dict_config_ini, "Video", "renderer", "Constants", "AUTO_RENDERER", AUTO_RENDERER);
	
	config.width = ini_getint(dict_config_ini, "Video", "screen_width", 640);
	config.height = ini_getint(dict_config_ini, "Video", "screen_height", 480);
	config.bits_per_pixel = ini_getint(dict_config_ini, "Video", "color_depth", 32);
	config.caption = ini_getstring(dict_config_ini, "Video", "caption", "JDV Bomberman");
	//config.palettes_location = ini_getstring(dict_config_ini, "Video", "palette_location", 0);
	
	config.num_players = ini_getint(dict_config_ini, "Game", "num_players", 0);
	if (config.num_players == 0)
		fprintf(stderr, "Check config.ini, [Game] num_players\n");
	for (i = 0; i < config.num_players; i++)
	{
		sprintf(buf, "Player%d", i);
		config.player_conf[i].key_up      = ini_getint_symbolic(dict_config_ini, buf, "key_up",      "Constants", 0, -1);
		config.player_conf[i].key_down    = ini_getint_symbolic(dict_config_ini, buf, "key_down",    "Constants", 0, -1);
		config.player_conf[i].key_left    = ini_getint_symbolic(dict_config_ini, buf, "key_left",    "Constants", 0, -1);
		config.player_conf[i].key_right   = ini_getint_symbolic(dict_config_ini, buf, "key_right",   "Constants", 0, -1);
		config.player_conf[i].key_action1 = ini_getint_symbolic(dict_config_ini, buf, "key_action1", "Constants", 0, -1);
		config.player_conf[i].key_action2 = ini_getint_symbolic(dict_config_ini, buf, "key_action2", "Constants", 0, -1);
		config.player_conf[i].color       = ini_getint_symbolic(dict_config_ini, buf, "color",       "Constants", 0, -1);
		config.player_conf[i].team        = ini_getint         (dict_config_ini, buf, "team", 0);
	}
	
	config.level_layout = ini_getint(dict_config_ini, "Level", "layout", -1); assert(config.level_layout != -1);
	config.level_scheme = ini_getstring(dict_config_ini, "Level", "level_scheme", 0); assert(config.level_scheme);
	config.draw_grid = ini_getint(dict_config_ini, "Video", "draw_grid", 0);
	config.starting_player_speed = ini_getfloat(dict_config_ini, "Game", "starting_player_speed", STARTING_PLAYER_SPEED);
	config.walk_anim_frame_distance = ini_getfloat(dict_config_ini, "Game", "walk_anim_frame_distance", WALK_ANIM_FRAME_DISTANCE);
	
	config.key_pause_time =  ini_getint_symbolic(dict_config_ini, "Game", "key_pause_time" , "Constants", 0, -1);
	config.key_bullet_time = ini_getint_symbolic(dict_config_ini, "Game", "key_bullet_time", "Constants", 0, -1);
	
	return 0;
}

int config_quit()
{
	int i;
	
	for (i = 0; i < ndummy; i++)
		free_ani(dummy[i]);
	
	// be careful as iniparser_freedict(dict) free's the returned strings
	// - make a copy with asprintf which automatically allocates space
	// - or wait till program quit to free it
	//iniparser_dump_ini(dict_config_ini, stderr);
	iniparser_freedict(dict_config_ini);
	
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

int RW_file_set(char *file)
{
	strcpy(RW_file, file);
	
	return 0;
}

char *ini_getstring(dictionary *dict, char *section, char *key, char *def_val)
{
	char *str;
	char *ret;
	
	asprintf(&str, "%s:%s", section, key);
	ret = iniparser_getstring(dict, str, def_val);
	free(str);
	
	return ret;
}

int ini_getint(dictionary *dict, char *section, char *key, int def_val)
{
	char *str;
	
	str = ini_getstring(dict, section, key, 0);
	if (!str)
	{
		return def_val;
	}
	
	return strtol(str, 0, 0);
}

float ini_getfloat(dictionary *dict, char *section, char *key, float def_val)
{
	char *str;
	
	str = ini_getstring(dict, section, key, 0);
	if (!str)
	{
		return def_val;
	}
	
	return strtof(str, 0);
}

char *ini_getstring_symbolic(dictionary *dict, char *section, char *key, char *symb_lookup_section, char *def_symb, char *def_val)
{
	char *str;
	
	str = ini_getstring(dict, section, key, def_symb);
	str = ini_getstring(dict, symb_lookup_section, str, def_val);
	
	return str;
}

int ini_getint_symbolic(dictionary *dict, char *section, char *key, char *symb_lookup_section, char *def_symb, int def_val)
{
	char *str;
	
	str = ini_getstring_symbolic(dict, section, key, symb_lookup_section, def_symb, 0);
	if (!str)
	{
		return def_val;
	}
	
	return strtol(str, 0, 0);
}

float ini_getfloat_symbolic(dictionary *dict, char *section, char *key, char *symb_lookup_section, char *def_symb, float def_val)
{
	char *str;
	
	str = ini_getstring_symbolic(dict, section, key, symb_lookup_section, def_symb, 0);
	if (!str)
	{
		return def_val;
	}
	
	return strtof(str, 0);
}
