/**
  @file    config.h
  @author  johndoevodka
  @date    Aug 2011
  @version 0.1
  @brief   Config parameters, some helper functions, loading stuff, initialising and exiting
  
  Configuration parameters are encapsulated inside struct config.
*/

#ifndef CONFIG_H
#define CONFIG_H
#ifdef __cplusplus
extern "C" {
#endif


#include "SDL.h"
// because of dictionary :>
#include "iniparser.h"


#define CONFIG_FILE                     "config.ini"
#define MAX_PLAYERS                     10
#define MAX_KEYBOARD_PLAYERS            4
#define LEVEL_ROWS                      11
#define LEVEL_COLS                      15
#define TILE_WIDTH                      40
#define TILE_HEIGHT                     36
#define LOWER_LEFT_CORNER_X             20
#define LOWER_LEFT_CORNER_Y             21
#define STARTING_PLAYER_SPEED           184.6
#define DRAWING_OFFSET_Y                (-(TILE_HEIGHT)/2)
#define FLAME_OFFSET_Y                  16


#define ANIM_ANGEL_SPEED                5.0f


// TODO: pimp next constant, it represents how much the bomberman must move
// (in pixels) so that the animation advances a frame
#define WALK_ANIM_FRAME_DISTANCE        2.0f
#define WALK_AROUND_THRESHOLD           0.1f

// if the following macro is defined, when walking around obstacles the direction
// will be 'fixed' (bomber will look in the right direction) in regards to the
// direction in the original game (Atomic Bomberman)
// if you want behaviour exactly as in the original game, do not define it
//#define WALK_AROUND_DIRECTION_FIX

// be careful when making the program sleep, as that fucks up time measuring
// or maybe it doesnt?
#define TIME_MEASURING_clock_gettime
//#define TIME_MEASURING_SDL_GetTicks

#define ANIM_FRAME_TIME                 0.05f
#define BOMB_ANIM_FRAME_TIME            0.05f
#define FLAME_ANIM_FRAME_TIME           0.05f
#define BOMB_TIMER                      2.0f
#define BOMB_TIMER_SHORT                1.0f
#define FLAME_TIMER                     0.5f
#define XBRICK_TIMER                    0.5f


#define PATH_PALETTES_LOCATION          "data/graphics/palettes"
#define PATH_ANIM_WALK                  "data/graphics/walk"
#define PATH_ANIM_PUNCH_NORTH           "data/graphics/punbomb2"
#define PATH_ANIM_PUNCH_SOUTH           "data/graphics/punbomb1"
#define PATH_ANIM_PUNCH_WEST            "data/graphics/punbomb3"
#define PATH_ANIM_PUNCH_EAST            "data/graphics/punbomb4"
#define PATH_IMAGE_LEVEL_BACKGROUND     "data/res/field%d.pcx"
#define PATH_ANIM_LEVEL_TILES           "data/graphics/tiles%d"
#define PATH_ANIM_LEVEL_XBRICK          "data/graphics/xbrick%d"
#define PATH_ANIM_SHADOW                "data/graphics/shadow"
#define PATH_ANIM_STAND                 "data/graphics/stand"
#define PATH_ANIM_LEVEL_EXTRAS          "data/graphics/extras"
#define PATH_ANIM_BOMB                  "data/graphics/bombs"
#define PATH_ANIM_TRIGGER               "data/graphics/triganim"
#define PATH_ANIM_LEVEL_CARRY           "data/graphics/conveyor"
#define PATH_ANIM_FLAME                 "data/graphics/mflame"
#define PATH_ANIM_XPLODE                "data/graphics/xplode%d"
#define PATH_ANIM_CORNER                "data/graphics/corner%d"
#define PATH_ANIM_POWERS                "data/graphics/powers"


#define COLOR_GRID                      0x30ff0000
#define COLOR_GRID_HALF                 0x20ff0000
#define COLOR_YELLOW                    0xffffff00
#define COLOR_MAGENTA                   0xffff00ff
#define COLOR_CYAN                      0xff00ffff


static inline int min(int x, int y) { return ((x) < (y) ? (x) : (y)); }
static inline int max(int x, int y) { return ((x) > (y) ? (x) : (y)); }


typedef enum _GRAPHICS_RENDERER {AUTO_RENDERER, OPENGL_RENDERER, SDL_RENDERER, NONE_RENDERER} GRAPHICS_RENDERER;

enum {HEADING_NORTH, HEADING_SOUTH, HEADING_WEST, HEADING_EAST};
enum {HEADING_UP,    HEADING_DOWN,  HEADING_LEFT, HEADING_RIGHT, HEADING_NONE};


typedef struct _PLAYER_CONF {
	SDLKey key_up;
	SDLKey key_down;
	SDLKey key_left;
	SDLKey key_right;
	SDLKey key_action1;
	SDLKey key_action2;
	int color;
	int team;
	int start_row;
	int start_col;
} PLAYER_CONF;

typedef struct _CONFIG {
	GRAPHICS_RENDERER graphics_renderer;
	GRAPHICS_RENDERER used_graphics_renderer;
	int width;
	int height;
	int bits_per_pixel;
	char *caption;
	
	int num_players;
	PLAYER_CONF player_conf[MAX_PLAYERS];
	float starting_player_speed;
	float walk_anim_frame_distance;
	
	int level_layout;
	int draw_grid;
	char *level_scheme;
	
	// as read from level scheme file
	int max_players_on_level;
	int starting_bombs;        int level_bombs;
	int starting_flame;        int level_flame;
	
	int starting_disease;      int level_disease;
	int starting_kicker;       int level_kicker;
	int starting_skates;       int level_skates;
	int starting_punch;        int level_punch;
	int starting_grab;         int level_grab;
	int starting_spooger;      int level_spooger;
	int starting_goldflame;    int level_goldflame;
	int starting_trigger;      int level_trigger;
	int starting_jelly;        int level_jelly;
	int starting_superdisease; int level_superdisease;
	int starting_random;       int level_random;
	
	SDLKey key_pause_time;
	SDLKey key_bullet_time;
} CONFIG;

extern CONFIG config;
extern float delta_time;
extern float delta_time_real;
extern int frames_per_second;


#include "graphics.h"
extern ANI *dummy[];
extern int ndummy;



char *ini_getstring(dictionary *dict, char *section, char *key, char *def_val);
int ini_getint(dictionary *dict, char *section, char *key, int def_val);
float ini_getfloat(dictionary *dict, char *section, char *key, float def_val);
char *ini_getstring_symbolic(dictionary *dict, char *section, char *key, char *symb_lookup_section, char *def_symb, char *def_val);
int ini_getint_symbolic(dictionary *dict, char *section, char *key, char *symb_lookup_section, char *def_symb, int def_val);
float ini_getfloat_symbolic(dictionary *dict, char *section, char *key, char *symb_lookup_section, char *def_symb, float def_val);

char* SDL_RWReadLine(char *buf, int size, SDL_RWops *rw);
char* SDL_RWReadLine_comment(char *buf, int size, SDL_RWops *rw);

extern char RW_file[4096];
char* RW_readline(SDL_RWops *rw);
int RW_scanf(SDL_RWops *rw, const char *format, ...);
int RW_file_set(char *file);

int config_init();
int config_quit();


#ifdef __cplusplus
}
#endif
#endif /* CONFIG_H */
