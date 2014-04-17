/**
  @file    graphics.c
  @author  ivansandrk
  @date    Aug 2011
  @version 0.1
  @brief   Middle layer that seperates game code and rendering code; it abstracts the rendering code.
  
  Uses OpenGL renderer as the backbone. (SDL renderer support is planned)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "opengl_renderer.h"
#include "sdl_renderer.h"
#include "graphics.h"
#include "SDL_rwops_zzip.h"


// each one is 256 x RGBA = 1024 bytes
static void *palette[10];

// dummy structure for saving ANI's
// -------make sure its only visible in this file - add static qualifier
// -------(had some nasty bug, it probably accessed the other file's variables)
// + now its all in one place (right here in graphics.c)
static ANI *loaded_ani[100];
static int n_loaded_ani = 0;


int graphics_init()
{
	int result = 0;
	
	switch (config.graphics_renderer) {
	case OPENGL_RENDERER:
	case AUTO_RENDERER:
		result = opengl_renderer_init(config.width, config.height, config.bits_per_pixel, WINDOW_CAPTION);
		
		if (result == -1)
		{
			fprintf(stderr, "Unable to use OpenGL renderer.\n");
			if (config.graphics_renderer == AUTO_RENDERER)
			{
				fprintf(stderr, "Trying fallback renderer instead - SDL renderer.\n");
			}
		}
		
		if (result == 0 || config.graphics_renderer == OPENGL_RENDERER)
		{
			config.graphics_renderer = OPENGL_RENDERER;
			break;
		}
	
	case SDL_RENDERER:
		result = sdl_renderer_init(config.width, config.height, config.bits_per_pixel, WINDOW_CAPTION, FONT_PATH);
		config.graphics_renderer = SDL_RENDERER;
		
		if (result == -1)
		{
			fprintf(stderr, "Unable to use SDL renderer.\n");
			config.graphics_renderer = NONE_RENDERER;
		}
		
		break;
	
	default:
		fprintf(stderr, "Unknown renderer in config.\n");
		result = -1;
	}
	
	if (result == 0)
	{
		result = load_palettes();
	}
	
	return result;
}

int graphics_quit()
{
	int i;
	
	if (config.graphics_renderer == OPENGL_RENDERER)
	{
		opengl_renderer_quit();
	}
	else if (config.graphics_renderer == SDL_RENDERER)
	{
		sdl_renderer_quit();
	}
	else if (config.graphics_renderer == NONE_RENDERER)
	{
		fprintf(stderr, "Trying to graphics_quit() with graphics_renderer = NONE_RENDERER.\n");
	}
	
	free_palettes();
	
	for (i = 0; i < n_loaded_ani; i++)
	{
		free_ani(loaded_ani[i]);
	}
	
	return 0;
}

Image* load_image(char *file, int hotspot_x, int hotspot_y, int colorkey_index)
{
	Image *im;
	SDL_RWops *data = SDL_RWFromZZIP(file, "r");
	
	if (!data)
	{
		fprintf(stderr, "Unable to open %s: %s\n", file, strerror(errno));
		return 0;
	}
	
	im = (Image*) malloc(sizeof(Image));
	im->colorkey_index = colorkey_index;
	im->hotspot_x = hotspot_x;
	im->hotspot_y = hotspot_y;
	
	if ((config.graphics_renderer == OPENGL_RENDERER && opengl_renderer_load_texture(data, im) == -1) ||
	    (config.graphics_renderer == SDL_RENDERER && sdl_renderer_load_image(data, im) == -1))
	{
		free(im);
		im = 0;
	}
	
	SDL_RWclose(data);
	
	return im;
}

Image* load_image_without_palette(char *file)
{
	Image *im;
	SDL_RWops *data = SDL_RWFromZZIP(file, "r");
	int fail = 1;
	
	if (!data)
	{
		fprintf(stderr, "Unable to open %s: %s\n", file, strerror(errno));
		return 0;
	}
	
	im = (Image*) malloc(sizeof(Image));
	im->colorkey_index = -1;
	
	if (config.graphics_renderer == OPENGL_RENDERER)
		fail = (opengl_renderer_load_texture_without_palette(data, im) == -1);
	else if (config.graphics_renderer == SDL_RENDERER)
		fail = (sdl_renderer_load_image(data, im) == -1);
	
	if (fail)
	{
		free(im);
		im = 0;
	}
	else
	{
		im->hotspot_x = 0;
		im->hotspot_y = im->height;
	}
	SDL_RWclose(data);
	
	return im;
}

int free_image(Image *im)
{
	if (im)
	{
		if (config.graphics_renderer == OPENGL_RENDERER)
			opengl_renderer_free_texture(im);
		else if (config.graphics_renderer == OPENGL_RENDERER)
			sdl_renderer_free_image(im);
		
		free(im);
		return 0;
	}
	
	return -1;
}

int draw(Image *im, int color, int x, int y)
{
	void *pal;
	
	if (color >= 0 && color <= 9)
		pal = palette[color];
	else
		pal = 0;
	
	x = x - im->hotspot_x;
	y = y + im->hotspot_y - im->height;
	
	if (config.graphics_renderer == OPENGL_RENDERER)
		opengl_renderer_draw(im, pal, x, y);
	else if (config.graphics_renderer == SDL_RENDERER)
		sdl_renderer_draw(im, pal, x, y);
	else
		return -1;
	
	return 0;
}

int draw_llc(Image *im, int color, int x, int y)
{
	return draw(im, color, LOWER_LEFT_CORNER_X + x, LOWER_LEFT_CORNER_Y + y);
}

int draw_tiled(Image *im, int color, int row, int col)
{
	int x, y;
	x = TILE_WIDTH  * col + TILE_WIDTH / 2;
	y = TILE_HEIGHT * row;
	
	return draw_llc(im, color, x, y);
}

int load_palettes()
{
	int i, j;
	SDL_RWops *rw;
	unsigned char tmp[3*256];
	
	/* all palettes are stored in one file, sequentially */
	
	if (!(rw = SDL_RWFromZZIP(PATH_PALETTES_LOCATION, "r")))
	{
		fprintf(stderr, "Unable to open palettes file: %s\n", strerror(errno));
		return -1;
	}
	
	for (i = 0; i < 10; i++)
	{
		palette[i] = malloc(256 * 4);
		// set alpha to full
		memset(palette[i], 255, 256 * 4);
		
		if (SDL_RWread(rw, tmp, 3, 256) < 256)
		{
			fprintf(stderr, "Not enough palettes data.\n");
			free_palettes();
			return -1;
		}
		
		for (j = 0; j < 256; j++)
		{
			memcpy((unsigned char*)palette[i]+4*j, tmp+3*j, 3);
		}
	}
	
	SDL_RWclose(rw);
	return 0;
}

int free_palettes()
{
	int i;
	
	for (i = 0; i < 10; i++)
		if (palette[i])
			free(palette[i]);
	
	return 0;
}

int drawing_begin()
{
	if (config.graphics_renderer == OPENGL_RENDERER)
		return opengl_renderer_drawing_begin();
	
	else if (config.graphics_renderer == SDL_RENDERER)
		return sdl_renderer_drawing_begin();
	
	return -1;
}

int drawing_end()
{
	if (config.graphics_renderer == OPENGL_RENDERER)
		return opengl_renderer_drawing_end();
	
	else if (config.graphics_renderer == SDL_RENDERER)
		return sdl_renderer_drawing_end();
	
	return -1;
}

int clear_screen()
{
	if (config.graphics_renderer == OPENGL_RENDERER)
		return opengl_renderer_clear_screen();
	
	else if (config.graphics_renderer == SDL_RENDERER)
		return sdl_renderer_clear_screen();
	
	return -1;
}

ANI* load_anim(const char *path)
{
	// headwipe ani has the most images (211)
	// maximum number of sequences is 14 for classic.ani and powers.ani
	// long ass buffer in case someone tries to use really long paths
	char buf1[2048];
	// longest line in */ani.ini is 22 (wc -L */ani.ini | sort)
	// 100 > 22
	char buf2[100];
	int i, j;
	SDL_RWops *rw_ini;
	int hotspot_x, hotspot_y, colorkey_index;
	int frame_index;
	ANI *ani;
	
	sprintf(buf1, "%s/ani.ini", path);
	rw_ini = SDL_RWFromZZIP(buf1, "r");
	RW_file_set(buf1);
	if (!rw_ini)
	{
		fprintf(stderr, "Unable to open %s: %s\n", buf1, strerror(errno));
		return 0;
	}
	
	ani = (ANI*) malloc(sizeof(ANI));
	RW_scanf(rw_ini, "%d", &ani->n_images);
	ani->im = (Image**) malloc(ani->n_images * sizeof(Image*));
	
	for (i = 0; i < ani->n_images; i++)
	{
		RW_scanf(rw_ini, "%s %d %d %d", buf2, &hotspot_x, &hotspot_y, &colorkey_index);
		sprintf(buf1, "%s/%s", path, buf2);
		ani->im[i] = load_image(buf1, hotspot_x, hotspot_y, colorkey_index);
		if (!(ani->im[i]))
		{
			return 0;
		}
	}
	
	RW_scanf(rw_ini, "%d", &ani->n_anim);
	ani->anim = (Anim**) malloc(ani->n_anim * sizeof(Anim*));
	
	for (i = 0; i < ani->n_anim; i++)
	{
		Anim *anim = ani->anim[i] = (Anim*) malloc(sizeof(Anim));
		sprintf(buf2, "%s", RW_readline(rw_ini));
		anim->name = (char*) malloc(strlen(buf2) + 1);
		strcpy(anim->name, buf2);
		RW_scanf(rw_ini, "%d", &anim->n_frames);
		anim->frames = (Frame*) malloc(anim->n_frames * sizeof(Frame));
		
		for (j = 0; j < anim->n_frames; j++)
		{
			RW_scanf(rw_ini, "%d %d %d", &frame_index, &anim->frames[j].x, &anim->frames[j].y);
			anim->frames[j].im = ani->im[frame_index];
		}
	}
	
	SDL_RWclose(rw_ini);
	
	return (loaded_ani[n_loaded_ani++] = ani);
}

int free_ani(ANI *ani)
{
	int i;
	
	if (!ani)
		return -1;
	
	if (ani->im)
	{
		for (i = 0; i < ani->n_images; i++)
			free_image(ani->im[i]);
		
		free(ani->im);
	}
	
	if (ani->anim)
	{
		for (i = 0; i < ani->n_anim; i++)
		{
			if (!ani->anim[i])
				continue;
			
			if (ani->anim[i]->frames)
				free(ani->anim[i]->frames);
			
			if (ani->anim[i]->name)
				free(ani->anim[i]->name);
			
			free(ani->anim[i]);
		}
		free(ani->anim);
	}
	
	free(ani);
	
	return 0;
}

int print(int x, int y, char *str)
{
	if (config.graphics_renderer == SDL_RENDERER)
		return sdl_renderer_print(x, y, str);
	
	return -1;
}


/** *************************************** hackity-hack-hack *****************************/
int draw_square(int x, int y, int size, int color)
{
	if (config.graphics_renderer != SDL_RENDERER)
		return -1;
	
	return sdl_renderer_draw_square(x+LOWER_LEFT_CORNER_X, y+LOWER_LEFT_CORNER_Y, size, color);
}

int draw_vline(int x, int y0, int y1, int color)
{
	if (config.graphics_renderer != SDL_RENDERER)
		return -1;
	
	return sdl_renderer_draw_vline(x+LOWER_LEFT_CORNER_X,
		y0+LOWER_LEFT_CORNER_Y, y1+LOWER_LEFT_CORNER_Y, color);
}

int draw_hline(int x0, int x1, int y, int color)
{
	if (config.graphics_renderer != SDL_RENDERER)
		return -1;
	
	return sdl_renderer_draw_hline(x0+LOWER_LEFT_CORNER_X,
		x1+LOWER_LEFT_CORNER_X, y+LOWER_LEFT_CORNER_Y, color);
}

int draw_grid()
{
	int i;
	
	// grid
	for (i = 0; i <= LEVEL_COLS; i++)
		draw_vline(i * TILE_WIDTH, 0, LEVEL_ROWS * TILE_HEIGHT, COLOR_GRID);
	
	for (i = 0; i <= LEVEL_ROWS; i++)
		draw_hline(0, LEVEL_COLS * TILE_WIDTH, i * TILE_HEIGHT, COLOR_GRID);
	
	// half grid
	for (i = 0; i < LEVEL_COLS; i++)
		draw_vline(i * TILE_WIDTH + TILE_WIDTH/2, 0, LEVEL_ROWS * TILE_HEIGHT, COLOR_GRID_HALF);
	
	for (i = 0; i < LEVEL_ROWS; i++)
		draw_hline(0, LEVEL_COLS * TILE_WIDTH, i * TILE_HEIGHT + TILE_HEIGHT/2, COLOR_GRID_HALF);
	
	return 0;
}

int draw_bounding_box(int x, int y, int width, int height, int color)
{
	draw_vline(x - width/2, y - height/2, y + height/2, color);
	draw_vline(x + width/2, y - height/2, y + height/2, color);
	draw_hline(x - width/2, x + width /2, y - height/2, color);
	draw_hline(x - width/2, x + width /2, y + height/2, color);
	
	return 0;
}
/** *************************************** hackity-hack-hack *****************************/

