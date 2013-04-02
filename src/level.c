

#include <errno.h>
#include <string.h>
#include "SDL_rwops_zzip.h"
#include <ctype.h>
#include <assert.h>

#include "config.h"
#include "level.h"
#include "graphics.h"


static Image *image_background;
static Image *image_tile_brick;
static Image *image_tile_solid;
static Image *image_arrow[4];
static Image *image_warp;

static Anim *anim_xbrick;
static Anim *anim_trampoline;
static Anim *anim_carry[4];


Field board[LEVEL_ROWS][LEVEL_COLS];
static int n_special;


static int level_read_scheme();
static int level_load_graphics();


int level_init()
{
	if (level_read_scheme() == -1 ||
	    level_load_graphics() == -1)
		return -1;
	
	return 0;
}

int level_quit()
{
	free_image(image_background);
	
	return 0;
}

int level_draw_all()
{
	int row, col;
	Image *im;
	
	draw(image_background, -1, 0, 0);
	
	for (row = 0; row < LEVEL_ROWS; row++)
	{
		for (col = 0; col < LEVEL_COLS; col++)
		{
			im = 0;
			if (board[row][col].type == LEVEL_TILE_BRICK)
				im = image_tile_brick;
			else if (board[row][col].type == LEVEL_TILE_SOLID)
				im = image_tile_solid;
			else if (board[row][col].type == LEVEL_TILE_XBRICK)
			{
				float *f = &board[row][col].special_anim_timer;
				
				*f += delta_time;
				if (*f >= XBRICK_TIMER)
				{
					board[row][col].type = LEVEL_TILE_EMPTY;
					*f = 0.0f;
				}
				else
					im = anim_xbrick->frames[(int)(*f / XBRICK_TIMER * anim_xbrick->n_frames)].im;
			}
			if (im != 0)
				goto jump_over_specials;
			if (board[row][col].special_type == LEVEL_SPECIAL_ARROW)
				im = image_arrow[board[row][col].special_info];
			else if (board[row][col].special_type == LEVEL_SPECIAL_TRAMPOLINE)
				im = anim_trampoline->frames[board[row][col].special_info].im;
			else if (board[row][col].special_type == LEVEL_SPECIAL_WARP)
				im = image_warp;
			else if (board[row][col].special_type == LEVEL_SPECIAL_CARRY)
			{
				const float n_f = 0.1f; // TODO: u config.h ili config.ini!!
				Anim *a = anim_carry[board[row][col].special_info];
				float *f = &board[row][col].special_anim_timer;
				*f += delta_time;
				while (*f >= n_f * a->n_frames)
					*f -= n_f * a->n_frames;
				im = a->frames[(int)(*f / n_f)].im;
			}
			else
				im = 0;
			
		jump_over_specials:
			if (im)
			{
				if (config.draw_grid)
					draw_bounding_box((col+0.5)*TILE_WIDTH,
					                  (row+0.5)*TILE_HEIGHT,
					                  TILE_WIDTH, TILE_HEIGHT, COLOR_CYAN);
				draw_tiled(im, -1, row, col);
			}
		}
	}
	
	return 0;
}

int level_brick_explode(int row, int col)
{
	if (board[row][col].type != LEVEL_TILE_BRICK)
		return -1;
	
	board[row][col].type = LEVEL_TILE_XBRICK;
	board[row][col].special_anim_timer = 0.0f;
	
	return 0;
}

int level_tile_is_passable(int row, int col)
{
	if (row < 0 || row >= LEVEL_ROWS || col < 0 || col >= LEVEL_COLS)
		return 0;
	
	if      (board[row][col].type == LEVEL_TILE_EMPTY  ||
	         board[row][col].type == LEVEL_TILE_FLAME)
		return 1;
	
	else if (board[row][col].type == LEVEL_TILE_BRICK  ||
	         board[row][col].type == LEVEL_TILE_SOLID  ||
	         board[row][col].type == LEVEL_TILE_XBRICK ||
	         board[row][col].type == LEVEL_TILE_BOMB)
		return 0;
	
	// should not get here
	assert("fix level_tile_is_passable" && 0);
}

static int level_load_graphics()
{
	ANI *ani;
	char buf[128];
	
	sprintf(buf, PATH_IMAGE_LEVEL_BACKGROUND, config.level_layout);
	if ((image_background = load_image_without_palette(buf)) == 0)
		return -1;
	
	sprintf(buf, PATH_ANIM_LEVEL_TILES, config.level_layout);
	if ((ani = load_anim(buf)) == 0)
		return -1;
	image_tile_brick = ani->anim[0]->frames[0].im;
	image_tile_solid = ani->anim[1]->frames[0].im;
	
	sprintf(buf, PATH_ANIM_LEVEL_XBRICK, config.level_layout);
	if ((ani = load_anim(buf)) == 0)
		return -1;
	anim_xbrick = ani->anim[0];
	
	if ((ani = load_anim(PATH_ANIM_LEVEL_EXTRAS)) == 0)
		return -1;
	anim_trampoline = ani->anim[5];
	image_warp = ani->anim[4]->frames[0].im;
	image_arrow[0] = ani->anim[0]->frames[0].im;
	image_arrow[1] = ani->anim[2]->frames[0].im;
	image_arrow[2] = ani->anim[3]->frames[0].im;
	image_arrow[3] = ani->anim[1]->frames[0].im;
	
	if ((ani = load_anim(PATH_ANIM_LEVEL_CARRY)) == 0)
		return -1;
	anim_carry[0] = ani->anim[2];
	anim_carry[1] = ani->anim[3];
	anim_carry[2] = ani->anim[0];
	anim_carry[3] = ani->anim[1];
	
	return 0;
}

// shave tiles around bomberman
int level_shave_tiles(int row, int col)
{
	if (board[row][col].type == LEVEL_TILE_BRICK)
		board[row][col].type = LEVEL_TILE_EMPTY;
	if (row-1 >= 0 && board[row-1][col].type == LEVEL_TILE_BRICK)
		board[row-1][col].type = LEVEL_TILE_EMPTY;
	if (row+1 < LEVEL_ROWS  && board[row+1][col].type == LEVEL_TILE_BRICK)
		board[row+1][col].type = LEVEL_TILE_EMPTY;
	if (col-1 >= 0 && board[row][col-1].type == LEVEL_TILE_BRICK)
		board[row][col-1].type = LEVEL_TILE_EMPTY;
	if (col+1 < LEVEL_COLS && board[row][col+1].type == LEVEL_TILE_BRICK)
		board[row][col+1].type = LEVEL_TILE_EMPTY;
	
	return 0;
}

static int level_read_scheme()
{
	int type;
	int row, col;
	char *buf;
	SDL_RWops *rw_level_scheme;
	int i;
	int special_info = 0;
	
	rw_level_scheme = SDL_RWFromZZIP(config.level_scheme, "r");
	RW_file_set(config.level_scheme);
	if (!rw_level_scheme)
	{
		fprintf(stderr, "Unable to open %s: %s\n", config.level_scheme, strerror(errno));
		return -1;
	}
	
	for (row = LEVEL_ROWS-1; row >= 0; row--)
	{
		buf = RW_readline(rw_level_scheme);
		
		for (col = 0; col < LEVEL_COLS; col++)
		{
			if (buf[col] == '#') // solid
				type = LEVEL_TILE_SOLID;
			else if (buf[col] == ':') // brick
				type = LEVEL_TILE_BRICK;
			else if (buf[col] == '.') // empty
				type = LEVEL_TILE_EMPTY;
			else
			{
				fprintf(stderr, "Something wrong with level data [%s], row (%d) col (%d).\n", config.level_scheme, row, col);
				return -1;
			}
			board[row][col].type = type;
		}
	}
	
	RW_scanf(rw_level_scheme, "%d", &n_special);
	
	for (i = 0; i < n_special; i++)
	{
		buf = RW_readline(rw_level_scheme);
		
		// lowercase to uppercase
		buf[0] = toupper(buf[0]);
		
		// arrow
		if (buf[0] == 'A')
		{
			sscanf(buf, "A %c %d %d", (char*)&special_info, &col, &row);
			board[row][col].special_type = LEVEL_SPECIAL_ARROW;
			special_info = toupper(special_info);
			if      (special_info == 'N') special_info = HEADING_NORTH;
			else if (special_info == 'S') special_info = HEADING_SOUTH;
			else if (special_info == 'W') special_info = HEADING_WEST;
			else if (special_info == 'E') special_info = HEADING_EAST;
			else assert(0);
			board[row][col].special_info = special_info;
		}
		// carry
		else if (buf[0] == 'C')
		{
			sscanf(buf, "C %c %d %d", (char*)&special_info, &col, &row);
			board[row][col].special_type = LEVEL_SPECIAL_CARRY;
			special_info = toupper(special_info);
			if      (special_info == 'N') special_info = HEADING_NORTH;
			else if (special_info == 'S') special_info = HEADING_SOUTH;
			else if (special_info == 'W') special_info = HEADING_WEST;
			else if (special_info == 'E') special_info = HEADING_EAST;
			else assert(0);
			board[row][col].special_info = special_info;
			board[row][col].special_anim_timer = 0.0f;
		}
		// trampoline
		else if (buf[0] == 'T')
		{
			sscanf(buf, "T %d %d", &col, &row);
			board[row][col].special_type = LEVEL_SPECIAL_TRAMPOLINE;
			board[row][col].special_info = 0;
		}
		// warp
		else if (buf[0] == 'W')
		{
			sscanf(buf, "W %d %d %d", &special_info, &col, &row);
			board[row][col].special_type = LEVEL_SPECIAL_WARP;
			board[row][col].special_info = special_info;
		}
		else assert(0);
	}
	
	RW_scanf(rw_level_scheme, "%d", &config.max_players_on_level);
	assert(config.max_players_on_level <= MAX_PLAYERS);
	assert(config.max_players_on_level >= config.num_players);
	
	for (i = 0; i < config.max_players_on_level; i++)
	{
		RW_scanf(rw_level_scheme, "%d %d", &config.player_conf[i].start_col, &config.player_conf[i].start_row);
	}
	
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_bombs,        &config.level_bombs);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_flame,        &config.level_flame);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_disease,      &config.level_disease);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_kicker,       &config.level_kicker);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_skates,       &config.level_skates);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_punch,        &config.level_punch);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_grab,         &config.level_grab);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_spooger,      &config.level_spooger);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_goldflame,    &config.level_goldflame);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_trigger,      &config.level_trigger);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_jelly,        &config.level_jelly);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_superdisease, &config.level_superdisease);
	RW_scanf(rw_level_scheme, "%d %d", &config.starting_random,       &config.level_random);
	
	return 0;
}

