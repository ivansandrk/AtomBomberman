/* sdl renderer for atomic bomberman clone
   - fully working, unlike opengl renderer
 */

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_gfxPrimitives.h"
#include "SDL_rwops_zzip.h"

//#include "config.h"
#include "sdl_renderer.h"


static SDL_Surface *screen;
static TTF_Font *font;
static int g_width, g_height;

int sdl_renderer_init(int width, int height, int bits_per_pixel, const char caption[], const char font_path[])
{
	g_width  = width;
	g_height = height;
	
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
	{
		fprintf(stderr, "Error initialising SDL Video subsystem: %s\n", SDL_GetError());
		return -1;
	}
	
	if ((screen = SDL_SetVideoMode(width, height, bits_per_pixel, SDL_HWSURFACE)) == 0)
	{
		fprintf(stderr, "Error setting Video Mode: %s\n", SDL_GetError());
		return -1;
	}
	
	SDL_WM_SetCaption(caption, 0);
	
	if (TTF_Init() == -1)
	{
		fprintf(stderr, "Error initializing SDL_ttf: %s\n", TTF_GetError());
		return -1;
	}
	
	SDL_RWops *data = SDL_RWFromZZIP(font_path, "r");
	font = TTF_OpenFontRW(data, 1, 12);
	if (font == 0)
	{
		fprintf(stderr, "Error loading font (%s) : %s\n", font_path, TTF_GetError());
		return -1;
	}
	
	return 0;
}

int sdl_renderer_quit()
{
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	
	return 0;
}

int sdl_renderer_load_image(SDL_RWops *data, Image *im)
{
	SDL_Surface *surf;
	
	if ((surf = IMG_Load_RW(data, 0)) == 0)
	{
		fprintf(stderr, "Unable to load image: %s\n", IMG_GetError());
		return -1;
	}
	
	im->surf   = surf;
	im->width  = surf->w;
	im->height = surf->h;
	if (im->colorkey_index >= 0 && im->colorkey_index <= 255)
		SDL_SetColorKey(surf, SDL_SRCCOLORKEY, im->colorkey_index);
	
	return 0;
}

int sdl_renderer_free_image(Image *im)
{
	SDL_FreeSurface(im->surf);
	
	return 0;
}

int sdl_renderer_draw(Image *im, void *pal, int x, int y)
{
	SDL_Rect rect = {(Sint16)x, (Sint16)(g_height-y-im->surf->h), 0, 0};
	
	if (pal)
	{
		SDL_SetPalette(im->surf, SDL_LOGPAL, (SDL_Color*) pal, 0, 256);
	}
	
	SDL_BlitSurface(im->surf, 0, screen, &rect);
	
	return 0;
}

int sdl_renderer_drawing_begin()
{
	//glClear(GL_COLOR_BUFFER_BIT);
	
	return 0;
}

int sdl_renderer_drawing_end()
{
	SDL_UpdateRect(screen, 0, 0, g_width, g_height);
	
	return 0;
}

int sdl_renderer_clear_screen()
{
	//glClear(GL_COLOR_BUFFER_BIT);
	//SDL_GL_SwapBuffers();
	
	return 0;
}

int sdl_renderer_print(int x, int y, char *str)
{
	SDL_Color color = {255, 0, 0, 255};
	SDL_Surface *surf_text;
	
	surf_text = TTF_RenderText_Blended(font, str, color);
	
	SDL_Rect rect = {(Sint16)x, (Sint16)(g_height-y-surf_text->h), 0, 0};
	SDL_BlitSurface(surf_text, 0, screen, &rect);
	SDL_FreeSurface(surf_text);
	
	return 0;
}

/** *************************************** hackity-hack-hack *****************************/
int sdl_renderer_draw_square(int x, int y, int size, int color)
{
	SDL_Rect rect = {(Sint16)(x-(size-1)), (Sint16)(g_height-y-(size-1)), (Uint16)(2*size-1), (Uint16)(2*size-1)};
	SDL_FillRect(screen, &rect, color);
	
	return 0;
}

int sdl_renderer_draw_vline(int x, int y0, int y1, int color)
{
	// change 0xAARRGGBB to 0xRRGGBBAA
	color = ((color & 0xff000000) >> 24) |
	        ((color & 0x00ffffff) << 8);
	
	vlineColor(screen, x, g_height - y1, g_height - y0, color);
	
	return 0;
}

int sdl_renderer_draw_hline(int x0, int x1, int y, int color)
{
	// change 0xAARRGGBB to 0xRRGGBBAA
	color = ((color & 0xff000000) >> 24) |
	        ((color & 0x00ffffff) << 8);
	
	hlineColor(screen, x0, x1, g_height - y, color);
	
	return 0;
}
/** *************************************** hackity-hack-hack *****************************/

