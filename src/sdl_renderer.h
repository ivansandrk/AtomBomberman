/* sdl renderer for atomic bomberman clone
   - for now this is just a dummy file, first get the opengl renderer working
 */


#ifndef SDL_RENDERER_H
#define SDL_RENDERER_H
#ifdef __cplusplus
extern "C" {
#endif


#include "SDL.h"
#include "graphics.h"

int sdl_renderer_init(int width, int height, int bits_per_pixel, char caption[]);
int sdl_renderer_quit();

int sdl_renderer_load_image(SDL_RWops *data, Image *im);
int sdl_renderer_free_image(Image *im);

int sdl_renderer_draw(Image *im, void *pal, int x, int y);

int sdl_renderer_drawing_begin();
int sdl_renderer_drawing_end();
int sdl_renderer_clear_screen();
int sdl_renderer_print(int x, int y, char *str);

int sdl_renderer_draw_square(int x, int y, int size, int color);
int sdl_renderer_draw_vline(int x, int y0, int y1, int color);
int sdl_renderer_draw_hline(int x0, int x1, int y, int color);

#ifdef __cplusplus
}
#endif
#endif /* SDL_RENDERER_H */

