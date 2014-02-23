/**
  @file    bomb.h
  @author  ivansandrk
  @date    Sep 2011
  @version 0.1
  @brief   bomb stuff
*/

#ifndef BOMB_H
#define BOMB_H

typedef struct _Bomb Bomb;
typedef struct _Flame Flame;

enum {BOMB_NORMAL, BOMB_JELLY, BOMB_TRIGGER};
enum {FLAME_CENTER, FLAME_UP_MID, FLAME_UP_TIP, FLAME_DOWN_MID, FLAME_DOWN_TIP, FLAME_LEFT_MID, FLAME_LEFT_TIP, FLAME_RIGHT_MID, FLAME_RIGHT_TIP, FLAME_NONE};

#include "graphics.h"

struct _Bomb {
	float x;
	float y;
	float elapsed_time;
	float timer;
	int color;
	int owner;
	int frame;
	int type;
	Anim *anim;
	
	int should_be_removed;
	int flame_len;
	//int activated;
};

struct _Flame {
	int row;
	int col;
	float elapsed_time;
	float timer;
	
	// owner and kill_owner can be different
	// if one players bomb activates other players bomb
	int color;
	//int owner;
	int kill_owner;
	
	// how much flame goes into each direction
	int n_up;
	int n_down;
	int n_left;
	int n_right;
	
	int frame;
};


int bomb_init();
int bomb_draw_all();
int bomb_process();
int bomb_quit();

#include "bomber.h"
// TODO: clean this stuff too?
int bomb_add_bomb(Bomber *b);


#endif /* BOMB_H */
