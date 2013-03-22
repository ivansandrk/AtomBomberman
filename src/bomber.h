/**
  @file    bomber.h
  @author  johndoevodka
  @date    Aug 2011
  @version 0.1
  @brief   bomber
*/

#ifndef BOMBER_H
#define BOMBER_H
#ifdef __cplusplus
extern "C" {
#endif

enum {BOMBER_STATE_WALKING, BOMBER_STATE_PUNCHING, BOMBER_STATE_KICKING,
      BOMBER_STATE_IDLE, BOMBER_STATE_CORNER, BOMBER_STATE_DEAD,
      BOMBER_STATE_DYING};

#include "graphics.h"

typedef struct _Bomber {
	int score;
	int kills;
	float x;
	float y;
	int color;
	int index;
	int team;
	
	// movement keys are set to 0/1 (according to actual state) on change
	// action keys are set to 0/1 on change (SDL_KEYUP / SDL_KEYDOWN)
	int move_up;
	int move_down;
	int move_left;
	int move_right;
	int action1;
	int action2;
	int auto_fire;
	
	int heading;
	int state;
	int frame;
	Anim *anim;
	float elapsed_time;
	
	// speed is in pixels/second
	float speed;
	float distance_travelled;
	
	int max_bombs;
	int used_bombs;
	int flame_len;
	
	int power_disease;
	int power_kicker;
	int power_skates;
	int power_punch;
	int power_grab;
	int power_spooger;
	int power_goldflame;
	int power_trigger;
	int power_jelly;
	int power_superdisease;
	int power_random;
	int power_bombs;
	int power_flame;
	
	int disease_slow;
} Bomber;

extern Bomber bombers[];
extern int n_bombers;

int bomber_init();
int bomber_draw_all();
int bomber_process();
int bomber_quit();
int bomber_draw_shadows();
int bomber_draw_bombers();


#ifdef __cplusplus
}
#endif
#endif /* BOMBER_H */
