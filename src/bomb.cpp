
#include <assert.h>
#include <math.h>
#include <vector>
using std::vector;

#include "config.h"
#include "graphics.h"
#include "level.h"
#include "bomb.h"
#include "bomber.h"
#include "sound.h"


static Anim *anim_bomb;
static Anim *anim_jelly;
static Anim *anim_trigger;
static Anim *anim_flame_center;
static Anim *anim_flame_up_mid;
static Anim *anim_flame_up_tip;
static Anim *anim_flame_down_mid;
static Anim *anim_flame_down_tip;
static Anim *anim_flame_left_mid;
static Anim *anim_flame_left_tip;
static Anim *anim_flame_right_mid;
static Anim *anim_flame_right_tip;
static Anim *anim_flame[9];

typedef vector<Bomb*> VecBomb;
typedef vector<Flame*> VecFlame;
static VecBomb g_bombs;
static VecFlame g_flames;


static int bomb_add_flame(Bomb *bomb, int kill_owner, int len[4]);
static int bomb_explode(Bomb *bomb, int kill_owner);

static int bomb_load_graphics()
{
	ANI *ani;
	// normal bombs & jelly bombs
	if ((ani = load_anim(PATH_ANIM_BOMB)) == 0)
		return -1;
	anim_bomb  = ani->anim[0];
	anim_jelly = ani->anim[1];
	
	// trigger bombs
	if ((ani = load_anim(PATH_ANIM_TRIGGER)) == 0)
		return -1;
	anim_trigger = ani->anim[0];
	
	// flame
	if ((ani = load_anim(PATH_ANIM_FLAME)) == 0)
		return -1;
	anim_flame[0] = anim_flame_center    = ani->anim[0];
	anim_flame[1] = anim_flame_up_mid    = ani->anim[5];
	anim_flame[2] = anim_flame_up_tip    = ani->anim[6];
	anim_flame[3] = anim_flame_down_mid  = ani->anim[7];
	anim_flame[4] = anim_flame_down_tip  = ani->anim[8];
	anim_flame[5] = anim_flame_left_mid  = ani->anim[1];
	anim_flame[6] = anim_flame_left_tip  = ani->anim[2];
	anim_flame[7] = anim_flame_right_mid = ani->anim[3];
	anim_flame[8] = anim_flame_right_tip = ani->anim[4];
	
	return 0;
}

int bomb_add_bomb(Bomber *b)
{
	Bomb *bomb;
	int row = b->y / TILE_HEIGHT;
	int col = b->x / TILE_WIDTH;
	
	// return if bomberman has no more bombs or theres already a bomb at the
	// current location
	if (b->used_bombs >= b->max_bombs ||
	    board[row][col].type == LEVEL_TILE_BOMB)
		return -1;
	
	bomb = (Bomb*) malloc(sizeof(Bomb));
	bomb->x = (col + 0.5f) * TILE_WIDTH;
	bomb->y = (row + 0.5f) * TILE_HEIGHT;
	bomb->elapsed_time = 0.0f;
	
	// TODO: pimp this here value, pass it from the caller as an argument
	// depending if the bomb timer is long (normal) or short (disease)
	// have the constants in config.ini?
	bomb->timer = BOMB_TIMER;
	bomb->owner = b->index;
	bomb->color = b->color;
	bomb->frame = 0;
	bomb->type = BOMB_NORMAL; // TODO: assign correct bomb type depending on what bomberman has
	bomb->anim = anim_bomb;
	
	bomb->should_be_removed = 0;
	bomb->flame_len = b->flame_len;
	//bomb->activated = 0;
	
	b->used_bombs++;
	if (board[row][col].type == LEVEL_TILE_EMPTY)
	{
		board[row][col].type = LEVEL_TILE_BOMB;
		board[row][col].type_info = bomb;
	}
	if (board[row][col].type == LEVEL_TILE_FLAME)
	{
		bomb_explode(bomb, ((Flame*)board[row][col].type_info)->kill_owner);
	}
	
	g_bombs.push_back(bomb);
	
	sound_play(SOUNDS_BOMB_DROP);
	
	return 0;
}

int bomb_init()
{
	if (bomb_load_graphics() == -1)
		return -1;
	
	return 0;
}

int bomb_quit()
{
	for (int i = 0; i < (int)g_bombs.size(); i++) {
		free(g_bombs[i]);
	}
	g_bombs.clear();
	
	for (int i = 0; i < (int)g_flames.size(); i++) {
		free(g_flames[i]);
	}
	g_flames.clear();
	
	return 0;
}

static int draw_flame(Anim *anim, Flame *flame, int row, int col)
{
	Frame *frame = &anim->frames[flame->frame];
	
	return draw_llc(
	frame->im,
	flame->color,
	(col + 0.5f) * TILE_WIDTH + frame->x,
	(row + 0.5f) * TILE_HEIGHT + DRAWING_OFFSET_Y - frame->y + FLAME_OFFSET_Y);
}

int bomb_draw_all()
{
	int row, col;
	Bomb *bomb;
	Flame *flame;
	
	for (VecBomb::iterator it = g_bombs.begin(); it != g_bombs.end(); it++) {
		bomb = *it;
		draw_llc(bomb->anim->frames[bomb->frame].im,
		         bomb->color,
		         bomb->x,
		         bomb->y + DRAWING_OFFSET_Y);
	}
	
	for (row = 0; row < LEVEL_ROWS; row++)
	for (col = 0; col < LEVEL_COLS; col++)
	{
		if (board[row][col].type != LEVEL_TILE_FLAME)
			continue;
		
		flame = (Flame*) board[row][col].type_info;
		
		draw_flame(anim_flame[board[row][col].flame_type], flame, row, col);
	}
	
	return 0;
}

int bomb_process()
{
	// for each bomb update timer, update frame (timer?), delete if necessary
	for (int i = 0; i < (int)g_bombs.size(); i++) {
		Bomb *bomb = g_bombs[i];
		bomb->elapsed_time += delta_time;
		
		if (bomb->elapsed_time >= bomb->timer)
		{
			bomb_explode(bomb, bomb->owner);
			continue;
		}
		
		bomb->frame = (int)(bomb->elapsed_time / ANIM_FRAME_TIME) % bomb->anim->n_frames;
	}
	
	// this should_be_removed piece of code is required because bomb_explode can remove
	// bomb's in random fashion - you could lose the next bomb in the list and then
	// all hell breaks loose
	for (VecBomb::iterator it = g_bombs.begin(); it != g_bombs.end(); ) {
		if ((*it)->should_be_removed) {
			free(*it);
			g_bombs.erase(it);
		}
		else {
			it++;
		}
	}
	
	for (VecFlame::iterator it = g_flames.begin(); it != g_flames.end(); ) {
		Flame *flame = *it;
		flame->elapsed_time += delta_time;
		
		if (flame->elapsed_time >= flame->timer)
		{
			// remove flame from tiles
			for (int d = 0; d < 4; d++)
				for (int i = 0; i <= flame->len[d]; i++) {
					int row = flame->row + i * heading_y[d];
					int col = flame->col + i * heading_x[d];
					if (board[row][col].type_info == flame) {
						board[row][col].type = LEVEL_TILE_EMPTY;
						board[row][col].type_info = NULL;
					}
				}
			
			free(flame);
			g_flames.erase(it);
			continue;
		}
		
		it++;
		// luckily, all the flame anim's have the same amount of frames
		flame->frame = (int)(flame->elapsed_time / ANIM_FRAME_TIME) % anim_flame_center->n_frames;
	}
	
	return 0;
}

static int bomb_explode(Bomb *bomb, int kill_owner)
{
	int i, d;
	int row = bomb->y / TILE_HEIGHT;
	int col = bomb->x / TILE_WIDTH;
	int len[4];
	
	bomb->should_be_removed = 1;
	bombers[bomb->owner].used_bombs--;
	if (board[row][col].type_info == bomb)
	{
		board[row][col].type = LEVEL_TILE_EMPTY;
		board[row][col].type_info = NULL;
	}
	
	for (d = 0; d < 4; d++) {
		int r, c;
		for (i = 0; 1; i++) {
			r = row + i * heading_y[d];
			c = col + i * heading_x[d];
			if (!level_tile_is_passable(r, c) || i > bomb->flame_len)
				break;
		}
		len[d] = max(0, i - 1);
		
		if (r >= 0 && r < LEVEL_ROWS && c >= 0 && c < LEVEL_COLS && i <= bomb->flame_len) {
			if (board[r][c].type == LEVEL_TILE_BOMB)
				bomb_explode((Bomb*)board[r][c].type_info, kill_owner);
			if (board[r][c].type == LEVEL_TILE_BRICK)
				level_brick_explode(r, c);
		}
	}
	
	bomb_add_flame(bomb, kill_owner, len);
	
	sound_play(SOUNDS_BOMB_EXPLODE);
	
	return 0;
}


static int bomb_add_flame(Bomb *bomb, int kill_owner, int len[4])
{
	int i, d;
	Flame *flame;
	
	flame = (Flame*) malloc(sizeof(Flame));
	flame->row = bomb->y / TILE_HEIGHT;
	flame->col = bomb->x / TILE_WIDTH;
	flame->elapsed_time = 0;
	flame->timer = FLAME_TIMER;
	
	flame->color = bomb->color;
	flame->kill_owner = kill_owner;
	
	// max(0, ...) prevents a certain "bug" when n_up=...=n_right=-1; it happens when the
	// bomb explodes on top of a BRICK - this normally shouldnt happen
	for (int d = 0; d < 4; d++)
		flame->len[d] = len[d];
	
	flame->frame = 0;
	
	for (d = 0; d < 4; d++) {
		for (i = 0; i <= flame->len[d]; i++) {
			int row = flame->row + i * heading_y[d];
			int col = flame->col + i * heading_x[d];
			board[row][col].type = LEVEL_TILE_FLAME;
			board[row][col].type_info = flame;
			board[row][col].flame_type = i >= bomb->flame_len ? flame_tip[d] :
			                             i > 0 ? flame_mid[d] : FLAME_CENTER;
		}
	}
	
	g_flames.push_back(flame);
	
	return 0;
}

