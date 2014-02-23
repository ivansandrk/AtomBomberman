
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "bomber.h"
#include "graphics.h"
#include "config.h"
#include "level.h"
#include "bomb.h"


// use Anim*,
// not Frame* (cant advance to next frame)
// not ANI* (not flexible enough - cant rearrange elements, cant combine Anim's from different files)
static Anim *anim_walk[4];
static Anim *anim_spin;
static Anim *anim_punch[4];
static Anim *anim_stand[4];
 
// [1, 17]
// 9 is angel
static int used_death_anims[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
//static int used_death_anims[] = {1, 2, 9};
//static int used_death_anims[] = {1};
static Anim *anim_xplode[17];

// [0, 7]
static int used_corner_anims[] = {0, 1, 2, 3, 4, 5, 6, 7};
//static int used_corner_anims[] = {0, 1, 2};
//static int used_corner_anims[] = {0};
static Anim *anim_corner[8];

// Image* is ok, for something simple like shadows
static Image *image_shadow;


// bug report - somehow n_bombers was changed to n_bombers-1
// i was playing with the add blocks routine (with the mouse)
// put blocks over everything, put bombs with all the bombers
// when i tried resurrecting them, bomber #3 (black) didnt come back
// then i noticed no info up there for the third bomber
// something to do with the green bomber #1 dying, when he is on a block
// sometime during his death, n_bombers gets messed with - bomber has to be on location 0,0
Bomber bombers[MAX_PLAYERS];
int n_bombers;


static int bomber_load_graphics()
{
	char buf[128];
	unsigned int i;
	ANI *ani;
	
	// walking
	if ((ani = load_anim(PATH_ANIM_WALK)) == 0)
		return -1;
	anim_walk[0] = ani->anim[0];
	anim_walk[1] = ani->anim[1];
	anim_walk[2] = ani->anim[2];
	anim_walk[3] = ani->anim[3];
	// spin
	anim_spin    = ani->anim[4];
	
	// punching
	anim_punch[0] = load_anim(PATH_ANIM_PUNCH_NORTH)->anim[0];
	anim_punch[1] = load_anim(PATH_ANIM_PUNCH_SOUTH)->anim[0];
	anim_punch[2] = load_anim(PATH_ANIM_PUNCH_WEST)->anim[0];
	anim_punch[3] = load_anim(PATH_ANIM_PUNCH_EAST)->anim[0];
	
	// shadow
	if ((ani = load_anim(PATH_ANIM_SHADOW)) == 0)
		return -1;
	image_shadow = ani->anim[0]->frames[0].im;
	
	// stand
	if ((ani = load_anim(PATH_ANIM_STAND)) == 0)
		return -1;
	anim_stand[0] = ani->anim[0];
	anim_stand[1] = ani->anim[1];
	anim_stand[2] = ani->anim[2];
	anim_stand[3] = ani->anim[3];
	
	
	// explode
	// anim 14. - 85 frames, 4 - 93, 1 - 83
	// TODO: takes too long to load, wonder why? profile!
	
	for (i = 0; i < sizeof(used_death_anims) / sizeof(used_death_anims[0]); i++) {
		sprintf(buf, PATH_ANIM_XPLODE, used_death_anims[i]); // [1, 17]
		if ((ani = load_anim(buf)) == 0)
			return -1;
		anim_xplode[i] = ani->anim[0];
	}
	
	
	// corner animations
	
	for (i = 0; i < sizeof(used_corner_anims) / sizeof(used_corner_anims[0]); i++) {
		sprintf(buf, PATH_ANIM_CORNER, used_corner_anims[i]);
		if ((ani = load_anim(buf)) == 0)
			return -1;
		anim_corner[i] = ani->anim[0];
	}
	
	return 0;
}

int bomber_init()
{
	int i;
	int perm[MAX_PLAYERS];
	
	if (bomber_load_graphics() == -1)
		return -1;
	
	n_bombers = config.num_players;
	
	// code used to permute player starting positions
	for (i = 0; i < MAX_PLAYERS; i++)
		perm[i] = i;
	for (i = 0; i < MAX_PLAYERS-1; i++)
	{
		int r = i + rand() % (MAX_PLAYERS - i);
		int t = perm[i];
		perm[i] = perm[r];
		perm[r] = t;
	}
	
	for (i = 0; i < n_bombers; i++)
	{
		bombers[i].score = 0;
		bombers[i].kills = 0;
		bombers[i].team = config.player_conf[i].team;
		bombers[i].color = config.player_conf[i].color;
		bombers[i].index = i;
		
		bombers[i].x = (config.player_conf[perm[i]].start_col + 0.5f) * TILE_WIDTH;
		bombers[i].y = (config.player_conf[perm[i]].start_row + 0.5f) * TILE_HEIGHT;
		level_shave_tiles(config.player_conf[perm[i]].start_row, config.player_conf[perm[i]].start_col);
		
		bombers[i].heading = HEADING_SOUTH;
		bombers[i].state = BOMBER_STATE_IDLE;
		bombers[i].frame = 0;
		bombers[i].anim = anim_stand[HEADING_SOUTH];
		
		bombers[i].speed = config.starting_player_speed;
		bombers[i].distance_travelled = 0.0f;
		
		bombers[i].max_bombs = config.starting_bombs;
		bombers[i].used_bombs = 0;
		bombers[i].flame_len = config.starting_flame;
	}
	
	return 0;
}

int bomber_quit()
{
	
	return 0;
}

int bomber_draw_shadows()
{
	int i;
	
	for (i = 0; i < n_bombers; i++)
	{
		if (bombers[i].state == BOMBER_STATE_DEAD ||
		    bombers[i].state == BOMBER_STATE_DYING)
			continue;
		
		draw_llc(image_shadow, -1, bombers[i].x,
		         bombers[i].y + DRAWING_OFFSET_Y);
	}
	
	return 0;
}

int bomber_draw_bombers()
{
	int i;
	char buf[128];
	
	for (i = 0; i < n_bombers; i++)
	{
		// draw score & kills (TODO - clean this up)
		sprintf(buf, "S:%d K:%d", bombers[i].score, bombers[i].kills);
		print(10+i/2*70, 460-(i%2)*20, buf);
		
		if (!bombers[i].anim)
			continue;
		
		// draw bomber
		draw_llc(bombers[i].anim->frames[bombers[i].frame].im,
		         bombers[i].color,
		         bombers[i].x + (0)*bombers[i].anim->frames[bombers[i].frame].x,
		         bombers[i].y + (1)*DRAWING_OFFSET_Y + (0)*bombers[i].anim->frames[bombers[i].frame].y);
		
		// debugging code
		if (config.draw_grid)
		{
			draw_bounding_box(bombers[i].x, bombers[i].y, TILE_WIDTH, TILE_HEIGHT, COLOR_YELLOW);
			
			draw_square(bombers[i].x, bombers[i].y, 2, COLOR_MAGENTA);
			draw_square(bombers[i].x, bombers[i].y, 1, COLOR_YELLOW);
			draw_square(bombers[i].x, bombers[i].y + DRAWING_OFFSET_Y, 2, COLOR_YELLOW);
			draw_square(bombers[i].x, bombers[i].y + DRAWING_OFFSET_Y, 1, COLOR_MAGENTA);
		}
	}
	
	
	return 0;
}

int bomber_draw_all()
{
	// this function aint called at all
	// because drawing order: shadows -> bombs -> bombers
	
	bomber_draw_shadows();
	bomber_draw_bombers();
	
	return 0;
}


static int heading_dx[] = {0,  0, -1, 1};
static int heading_dy[] = {1, -1,  0, 0};

// return value >= 0 (usually)
// but if bomberman just moved away from a occupied tile (when he drops a bomb)
// return value < 0
static float bomber_how_much_can_move(float x, float y, int direction)
{
	int row, col;
	float x0, y0;
	float dx, dy;
	
	row = y / TILE_HEIGHT;
	col = x / TILE_WIDTH;
	
	// if given bogus x or y or direction
	if (direction == HEADING_NONE || row < 0 || row >= LEVEL_ROWS || col < 0 || col >= LEVEL_COLS)
		return 0.0f;
	
	// move tile by tile until you hit an unpassable tile
	do {
		row += heading_dy[direction];
		col += heading_dx[direction];
	} while (level_tile_is_passable(row, col));
	
	// go back one tile
	row -= heading_dy[direction];
	col -= heading_dx[direction];
	
	x0 = (col + 0.5f) * TILE_WIDTH;
	y0 = (row + 0.5f) * TILE_HEIGHT;
	dx = x0 - x;
	dy = y0 - y;
	
	return dx * heading_dx[direction] + dy * heading_dy[direction];
}

static int bomber_correct(float *pxy, int tile_measure, float speed)
{
	float delta;
	
	delta = *pxy - ((int)*pxy / tile_measure + 0.5f) * tile_measure;
	
	if (delta > 0.0f)
	{
		*pxy -= fminf(speed * delta_time, delta);
	}
	else if (delta < 0.0f)
	{
		*pxy += fminf(speed * delta_time, -delta);
	}
	
	return 0;
}

static int bomber_advance(Bomber *b, int direction, float distance)
{
	if (direction == HEADING_NONE)// || distance <= 0.0f)
	{
		fprintf(stderr, "bomber_advance WTF %d %f\n", direction, distance);
		return -1;
	}
	
	if (direction == HEADING_UP)
		b->y += fminf(b->speed * delta_time, distance);
	else if (direction == HEADING_DOWN)
		b->y -= fminf(b->speed * delta_time, distance);
	else if (direction == HEADING_LEFT)
		b->x -= fminf(b->speed * delta_time, distance);
	else if (direction == HEADING_RIGHT)
		b->x += fminf(b->speed * delta_time, distance);
	
	if      (direction == HEADING_UP || direction == HEADING_DOWN)
		bomber_correct(&b->x, TILE_WIDTH , b->speed);
	else if (direction == HEADING_RIGHT || direction == HEADING_LEFT)
		bomber_correct(&b->y, TILE_HEIGHT, b->speed);
	
	return 0;
}

static int bomber_walk(Bomber *b)
{
	float dist1, dist2;
	int move1 = HEADING_NONE, move2 = HEADING_NONE;
	int actual_move = HEADING_NONE;
	
	if (b->move_up)
	{
		move2 = move1; move1 = HEADING_UP;
	}
	if (b->move_right)
	{
		move2 = move1; move1 = HEADING_RIGHT;
	}
	if (b->move_down)
	{
		move2 = move1; move1 = HEADING_DOWN;
	}
	if (b->move_left)
	{
		move2 = move1; move1 = HEADING_LEFT;
	}
	
	if (move1 == HEADING_NONE)
	{
		return HEADING_NONE;
	}
	
	// advance x, y
	dist1 = bomber_how_much_can_move(b->x, b->y, move1);
	dist2 = (move2 == HEADING_NONE ? 0.0f : bomber_how_much_can_move(b->x, b->y, move2));
	
	// TODO: if you want behaviour exactly as in the original, put != here (instead of >)
	// not sure if smart idea, some things may break
	// test it out!
	if (dist1 != 0.0f)
	{
		bomber_advance(b, move1, dist1);
		actual_move = move1;
	}
	else if (dist2 > 0.0f)
	{
		bomber_advance(b, move2, dist2);
		actual_move = move2;
	}
	else
	{
		actual_move = move1;
		
		if (move1 == HEADING_UP || move1 == HEADING_DOWN)
		{
			float dx = b->x - ((int)b->x / TILE_WIDTH + 0.5f) * TILE_WIDTH;
			if (dx > WALK_AROUND_THRESHOLD)
			{
				dist1 = !level_tile_is_passable((int)b->y/TILE_HEIGHT, (int)b->x/TILE_WIDTH + 1) ?
				        0.0f :
				        bomber_how_much_can_move(b->x + TILE_WIDTH, b->y, move1);
				if (dist1 > 0.0f)
				{
#					ifdef WALK_AROUND_DIRECTION_FIX
					actual_move = HEADING_RIGHT;
#					endif
					bomber_advance(b, HEADING_RIGHT, dist1);
				}
			}
			else if (dx < -WALK_AROUND_THRESHOLD)
			{
				dist1 = !level_tile_is_passable((int)b->y/TILE_HEIGHT, (int)b->x/TILE_WIDTH - 1) ?
				        0.0f :
				        bomber_how_much_can_move(b->x - TILE_WIDTH, b->y, move1);
				if (dist1 > 0.0f)
				{
#					ifdef WALK_AROUND_DIRECTION_FIX
					actual_move = HEADING_LEFT;
#					endif
					bomber_advance(b, HEADING_LEFT, dist1);
				}
			}
		}
		else if (move1 == HEADING_LEFT || move1 == HEADING_RIGHT)
		{
			float dy = b->y - ((int)b->y / TILE_HEIGHT + 0.5f) * TILE_HEIGHT;
			if (dy > WALK_AROUND_THRESHOLD)
			{
				dist1 = !level_tile_is_passable((int)b->y/TILE_HEIGHT + 1, (int)b->x/TILE_WIDTH) ?
				        0.0f :
				        bomber_how_much_can_move(b->x, b->y + TILE_HEIGHT, move1);
				if (dist1 > 0.0f)
				{
#					ifdef WALK_AROUND_DIRECTION_FIX
					actual_move = HEADING_UP;
#					endif
					bomber_advance(b, HEADING_UP, dist1);
				}
			}
			else if (dy < -WALK_AROUND_THRESHOLD)
			{
				dist1 = !level_tile_is_passable((int)b->y/TILE_HEIGHT - 1, (int)b->x/TILE_WIDTH) ?
				        0.0f :
				        bomber_how_much_can_move(b->x, b->y - TILE_HEIGHT, move1);
				if (dist1 > 0.0f)
				{
#					ifdef WALK_AROUND_DIRECTION_FIX
					actual_move = HEADING_DOWN;
#					endif
					bomber_advance(b, HEADING_DOWN, dist1);
				}
			}
		}
	}
	
	return actual_move;
}

static int bomber_do_states(Bomber *b)
{
	int i;
	int move;
	int b_row = b->y / TILE_HEIGHT;
	int b_col = b->x / TILE_WIDTH;
	Flame *flame;
	
	// TODO: add anim_state - have state & anim_state ???
	
	if (b->state == BOMBER_STATE_DEAD)
	{
	}
	
	// bomber can get killed from any state
	else if (b->state != BOMBER_STATE_DYING &&
	         board[b_row][b_col].type == LEVEL_TILE_FLAME)
	{
		b->state = BOMBER_STATE_DYING;
		
		i = rand() % (sizeof(used_death_anims) / sizeof(used_death_anims[0]));
		b->frame = 0;
		b->anim  = anim_xplode[i];
		b->elapsed_time = 0.0f;
		
		// add to kills (or subtract if suicide)
		flame = (Flame*) board[b_row][b_col].type_info;
		bombers[flame->kill_owner].kills += flame->kill_owner != b->index ? 1 : -1;
	}
	
	else if (b->state == BOMBER_STATE_CORNER)
	{
		move = bomber_walk(b);
		if (move != HEADING_NONE)
			b->heading = move;
		
		b->elapsed_time += delta_time;
		while (b->elapsed_time >= ANIM_FRAME_TIME)
		{
			b->elapsed_time -= ANIM_FRAME_TIME;
			b->frame++;
			if (b->frame == b->anim->n_frames)
				b->frame = 0;
		}
		
		if (level_tile_is_passable(b_row + 1, b_col    ) ||
		    level_tile_is_passable(b_row - 1, b_col    ) ||
		    level_tile_is_passable(b_row    , b_col + 1) ||
		    level_tile_is_passable(b_row    , b_col - 1)  )
		{
			b->state = BOMBER_STATE_IDLE;
			b->anim = anim_stand[b->heading];
			b->frame = 0;
		}
	}
	
	else if (b->state == BOMBER_STATE_IDLE)
	{
		// decouple movement and animation (this here is the movement part)
		move = bomber_walk(b);
		
		// if bomber gets trapped go into corner state
		if (!level_tile_is_passable(b_row + 1, b_col    ) &&
			!level_tile_is_passable(b_row - 1, b_col    ) &&
			!level_tile_is_passable(b_row    , b_col + 1) &&
			!level_tile_is_passable(b_row    , b_col - 1)  )
		{
			b->state = BOMBER_STATE_CORNER;
			i = rand() % (sizeof(used_corner_anims) / sizeof(used_corner_anims[0]));
			b->anim = anim_corner[i];
			b->frame = 0;
			b->elapsed_time = 0.0f;
		}
		
		// was idle, movement key pressed - start moving
		else if (move != HEADING_NONE)
		{
			b->state = BOMBER_STATE_WALKING;
			b->heading = move;
			b->anim = anim_walk[b->heading];
			b->frame = 0;
			b->distance_travelled = 0;
			
			// advance the animation variable
			b->distance_travelled += b->speed * delta_time;
		}
		
		// was idle, movement key not pressed - do nothing
		else if (move == HEADING_NONE)
		{
			// some other stuff comes here later, so dont just return
		}
	}
	
	else if (b->state == BOMBER_STATE_WALKING)
	{
		move = bomber_walk(b);
		
		// if bomber gets trapped go into corner state
		if (!level_tile_is_passable(b_row + 1, b_col    ) &&
			!level_tile_is_passable(b_row - 1, b_col    ) &&
			!level_tile_is_passable(b_row    , b_col + 1) &&
			!level_tile_is_passable(b_row    , b_col - 1)  )
		{
			b->state = BOMBER_STATE_CORNER;
			i = rand() % (sizeof(used_corner_anims) / sizeof(used_corner_anims[0]));
			b->anim = anim_corner[i];
			b->frame = 0;
			b->elapsed_time = 0.0f;
		}
		
		// was walking, movement key not pressed - go idle
		else if (move == HEADING_NONE)
		{
			b->state = BOMBER_STATE_IDLE;
			b->anim = anim_stand[b->heading];
			b->frame = 0;
		}
		
		// was walking, movement key pressed - advance moving animation
		else if (move != HEADING_NONE)
		{
			// still walking in the same direction
			if (b->heading == move)
			{
				while (b->distance_travelled > config.walk_anim_frame_distance)
				{
					b->distance_travelled -= config.walk_anim_frame_distance;
					
					b->frame++;
					if (b->frame == b->anim->n_frames)
						b->frame = 0;
				}
			}
			
			// new direction
			else if (b->heading != move)
			{
				b->heading = move;
				b->anim = anim_walk[b->heading];
				b->frame = 0;
				b->distance_travelled = 0;
			}
			
			// advance the animation variable
			b->distance_travelled += b->speed * delta_time;
		}
	}
	
	else if (b->state == BOMBER_STATE_DYING)
	{
		// i dont like this variant, the animation is "too smooth"
		if (0 && strcmp(b->anim->name, "die green 9\n") == 0)
			b->y += 100.0f * delta_time;
		
		b->elapsed_time += delta_time;
		while (b->elapsed_time >= ANIM_FRAME_TIME)
		{
			b->elapsed_time -= ANIM_FRAME_TIME;
			b->frame++;
			
			// angel animation - move sprite up
			if (strcmp(b->anim->name, "die green 9\n") == 0)
				b->y += ANIM_ANGEL_SPEED;
		}
		
		// explode anim over, switch state
		if (b->frame >= b->anim->n_frames)
		{
			if (strcmp(b->anim->name, "die green 9\n") == 0)
				b->y -= ANIM_ANGEL_SPEED * b->frame;
			b->anim = 0;
			b->state = BOMBER_STATE_DEAD;
		}
	}
	
	
	// respawn baby :-)
	if (b->action2)
	{
		b->action2 = 0;
		//b->auto_fire = !b->auto_fire;
		
		if (b->state == BOMBER_STATE_DYING || b->state == BOMBER_STATE_DEAD)
		{
			if (b->state == BOMBER_STATE_DYING &&
			    strcmp(b->anim->name, "die green 9\n") == 0)
				b->y -= ANIM_ANGEL_SPEED * b->frame;
			b->state = BOMBER_STATE_IDLE;
			b->anim = anim_stand[b->heading];
			b->frame = 0;
		}
	}
	
	if ((b->action1 || b->auto_fire)&&
	    (b->state == BOMBER_STATE_WALKING ||
	     b->state == BOMBER_STATE_IDLE ||
	     b->state == BOMBER_STATE_CORNER))
	{
		// we need to set this manually back to 0, input code does it but only
		// when the key is released, and this key should behave like a trigger
		b->action1 = 0;
		
		bomb_add_bomb(b);
	}
	
	return 0;
}

int bomber_process()
{
	int i;
	
	for (i = 0; i < n_bombers; i++)
	{
		bomber_do_states(&bombers[i]);
	}
	
	return 0;
}

