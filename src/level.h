/**
  @file    level.h
  @author  johndoevodka
  @date    Aug 2011
  @version 0.1
  @brief   level stuff
*/

#ifndef LEVEL_H
#define LEVEL_H
#ifdef __cplusplus
extern "C" {
#endif


#include "config.h"

enum {LEVEL_TILE_BRICK, LEVEL_TILE_SOLID, LEVEL_TILE_EMPTY, LEVEL_TILE_BOMB, LEVEL_TILE_FLAME, LEVEL_TILE_XBRICK};
enum {LEVEL_SPECIAL_NONE, LEVEL_SPECIAL_ARROW, LEVEL_SPECIAL_CARRY, LEVEL_SPECIAL_TRAMPOLINE, LEVEL_SPECIAL_WARP};

typedef struct _Field {
	int type;
	
	int special_type;
	int special_info;
	float special_anim_timer;
	
	// pointer to bomb/flame residing on that tile
	void *type_info;
	
	int flame_type;
} Field;



int level_init();
int level_quit();
int level_draw_all();
int level_tile_is_passable(int row, int col);
int level_brick_explode(int row, int col);
int level_shave_tiles(int row, int col);

extern Field board[LEVEL_ROWS][LEVEL_COLS];



#ifdef __cplusplus
}
#endif
#endif /* LEVEL_H */
