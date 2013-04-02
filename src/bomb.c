
#include <assert.h>
#include <math.h>

#include "config.h"
#include "graphics.h"
#include "level.h"
#include "bomb.h"
#include "bomber.h"
#include "utlist.h"
#include "sound.h"


static Anim *anim_bomb;
static Anim *anim_jelly;
static Anim *anim_trigger;
//static Image *image_shadow;
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

static Bomb  *head_bomb = 0;
static Flame *head_flame = 0;


static int bomb_add_flame(Bomb *bomb, int kill_owner, int n_up, int n_down, int n_left, int n_right);
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
	
	bomb = malloc(sizeof(Bomb));
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
	
	DL_APPEND(head_bomb, bomb);
	
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
	Bomb *bomb, *tmp_bomb;
	Flame *flame, *tmp_flame;
	
	DL_FOREACH_SAFE(head_bomb, bomb, tmp_bomb)
	{
		DL_DELETE(head_bomb, bomb);
		free(bomb);
	}
	
	DL_FOREACH_SAFE(head_flame, flame, tmp_flame)
	{
		DL_DELETE(head_flame, flame);
		free(flame);
	}
	
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
	
	DL_FOREACH(head_bomb, bomb)
	{
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
		
		flame = board[row][col].type_info;
		
		draw_flame(anim_flame[board[row][col].flame_type], flame, row, col);
	}
	
	return 0;
}

int bomb_process()
{
	int i;
	Bomb *bomb, *tmp_bomb;
	Flame *flame, *tmp_flame;
	
	// for each bomb update timer, update frame (timer?), delete if necessary
	DL_FOREACH(head_bomb, bomb)
	{
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
	DL_FOREACH_SAFE(head_bomb, bomb, tmp_bomb)
	{
		if (bomb->should_be_removed)
		{
			DL_DELETE(head_bomb, bomb);
			free(bomb);
		}
	}
	
	DL_FOREACH_SAFE(head_flame, flame, tmp_flame)
	{
		flame->elapsed_time += delta_time;
		
		if (flame->elapsed_time >= flame->timer)
		{
			DL_DELETE(head_flame, flame);
			
			if (board[flame->row][flame->col].type_info == flame)
			{
				board[flame->row][flame->col].type = LEVEL_TILE_EMPTY;
				board[flame->row][flame->col].type_info = 0;
			}
			
			for (i = 1; i <= flame->n_up; i++)
			if (board[flame->row + i][flame->col].type_info == flame)
			{
				board[flame->row + i][flame->col].type = LEVEL_TILE_EMPTY;
				board[flame->row + i][flame->col].type_info = 0;
			}
			
			for (i = 1; i <= flame->n_down; i++)
			if (board[flame->row - i][flame->col].type_info == flame)
			{
				board[flame->row - i][flame->col].type = LEVEL_TILE_EMPTY;
				board[flame->row - i][flame->col].type_info = 0;
			}
			
			for (i = 1; i <= flame->n_left; i++)
			if (board[flame->row][flame->col - i].type_info == flame)
			{
				board[flame->row][flame->col - i].type = LEVEL_TILE_EMPTY;
				board[flame->row][flame->col - i].type_info = 0;
			}
			
			for (i = 1; i <= flame->n_right; i++)
			if (board[flame->row][flame->col + i].type_info == flame)
			{
				board[flame->row][flame->col + i].type = LEVEL_TILE_EMPTY;
				board[flame->row][flame->col + i].type_info = 0;
			}
			
			free(flame);
			continue;
		}
		
		// luckily, all the flame anim's have the same amount of frames
		flame->frame = (int)(flame->elapsed_time / ANIM_FRAME_TIME) % anim_flame_center->n_frames;
	}
	
	return 0;
}

static int bomb_explode(Bomb *bomb, int kill_owner)
{
	int i;
	int row = bomb->y / TILE_HEIGHT, col = bomb->x / TILE_WIDTH;
	int n_up, n_down, n_left, n_right;
	
	bomb->should_be_removed = 1;
	bombers[bomb->owner].used_bombs--;
	if (board[row][col].type_info == bomb)
	{
		board[row][col].type = LEVEL_TILE_EMPTY;
		board[row][col].type_info = 0;
	}
	
	
	for (i = row; i < LEVEL_ROWS && level_tile_is_passable(i, col) && i-row <= bomb->flame_len; i++) {}
	n_up = i - row - 1;
	
	if (i < LEVEL_ROWS && i-row <= bomb->flame_len)
	{
		if      (board[i][col].type == LEVEL_TILE_BOMB)
			bomb_explode(board[i][col].type_info, kill_owner);
		else if (board[i][col].type == LEVEL_TILE_BRICK)
			level_brick_explode(i, col);
	}
	
	for (i = row; i >= 0 && level_tile_is_passable(i, col) && row-i <= bomb->flame_len; i--) {}
	n_down = row - i - 1;
	
	if (i >= 0 && row-i <= bomb->flame_len)
	{
		if      (board[i][col].type == LEVEL_TILE_BOMB)
			bomb_explode(board[i][col].type_info, kill_owner);
		else if (board[i][col].type == LEVEL_TILE_BRICK)
			level_brick_explode(i, col);
	}
	
	for (i = col; i >= 0 && level_tile_is_passable(row, i) && col-i <= bomb->flame_len; i--) {}
	n_left = col - i - 1;
	
	if (i >= 0 && col-i <= bomb->flame_len)
	{
		if      (board[row][i].type == LEVEL_TILE_BOMB)
			bomb_explode(board[row][i].type_info, kill_owner);
		else if (board[row][i].type == LEVEL_TILE_BRICK)
			level_brick_explode(row, i);
	}
	
	for (i = col; i < LEVEL_COLS && level_tile_is_passable(row, i) && i-col <= bomb->flame_len; i++) {}
	n_right = i - col - 1;
	
	if (i < LEVEL_COLS && i-col <= bomb->flame_len)
	{
		if      (board[row][i].type == LEVEL_TILE_BOMB)
			bomb_explode(board[row][i].type_info, kill_owner);
		else if (board[row][i].type == LEVEL_TILE_BRICK)
			level_brick_explode(row, i);
	}
	
	bomb_add_flame(bomb, kill_owner, n_up, n_down, n_left, n_right);
	
	sound_play(SOUNDS_BOMB_EXPLODE);
	
	return 0;
}


static int bomb_add_flame(Bomb *bomb, int kill_owner, int n_up, int n_down, int n_left, int n_right)
{
	int i;
	Flame *flame;
	
	flame = malloc(sizeof(Flame));
	flame->row = bomb->y / TILE_HEIGHT;
	flame->col = bomb->x / TILE_WIDTH;
	flame->elapsed_time = 0;
	flame->timer = FLAME_TIMER;
	
	flame->color = bomb->color;
	flame->kill_owner = kill_owner;
	
	// max(0, ...) prevents a certain "bug" when n_up=...=n_right=-1; it happens when the
	// bomb explodes on top of a BRICK - this normally shouldnt happen
	flame->n_up    = max(0, n_up);
	flame->n_down  = max(0, n_down);
	flame->n_left  = max(0, n_left);
	flame->n_right = max(0, n_right);
	
	flame->frame = 0;
	
	board[flame->row][flame->col].type = LEVEL_TILE_FLAME;
	board[flame->row][flame->col].type_info = flame;
	board[flame->row][flame->col].flame_type = FLAME_CENTER;
	for (i = 1; i <= flame->n_up; i++)
	{
		board[flame->row + i][flame->col].type = LEVEL_TILE_FLAME;
		board[flame->row + i][flame->col].type_info = flame;
		board[flame->row + i][flame->col].flame_type = (i < bomb->flame_len ? FLAME_UP_MID : FLAME_UP_TIP);
	}
	for (i = 1; i <= flame->n_down; i++)
	{
		board[flame->row - i][flame->col].type = LEVEL_TILE_FLAME;
		board[flame->row - i][flame->col].type_info = flame;
		board[flame->row - i][flame->col].flame_type = (i < bomb->flame_len ? FLAME_DOWN_MID : FLAME_DOWN_TIP);
	}
	for (i = 1; i <= flame->n_left; i++)
	{
		board[flame->row][flame->col - i].type = LEVEL_TILE_FLAME;
		board[flame->row][flame->col - i].type_info = flame;
		board[flame->row][flame->col - i].flame_type = (i < bomb->flame_len ? FLAME_LEFT_MID : FLAME_LEFT_TIP);
	}
	for (i = 1; i <= flame->n_right; i++)
	{
		board[flame->row][flame->col + i].type = LEVEL_TILE_FLAME;
		board[flame->row][flame->col + i].type_info = flame;
		board[flame->row][flame->col + i].flame_type = (i < bomb->flame_len ? FLAME_RIGHT_MID : FLAME_RIGHT_TIP);
	}
	
	DL_APPEND(head_flame, flame);
	
	return 0;
}

