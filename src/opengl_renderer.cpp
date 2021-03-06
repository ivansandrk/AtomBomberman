/**
  @file     opengl_renderer.c
  @author   ivansandrk
  @date     Aug 2011
  @version  $Revision: 0.1$
  @brief    OpenGL renderer for Atomic Bomberman clone
  
  Used to work, not working atm. Maybe it works some day again.
*/

#include <stdio.h>
#include "SDL.h"

#ifdef USE_OPENGL
#include "SDL_opengl.h"
#endif

#include "SDL_image.h"
#include "opengl_renderer.h"


int opengl_renderer_init(int width, int height, int bits_per_pixel, const char caption[])
{
#ifdef USE_OPENGL
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1)
	{
		fprintf(stderr, "opengl_renderer_init: %s\n", SDL_GetError());
		return -1;
	}
	
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	if (SDL_SetVideoMode(width, height, bits_per_pixel, SDL_OPENGL) == 0) // |SDL_FULLSCREEN
	{
		fprintf(stderr, "opengl_renderer_init: %s\n", SDL_GetError());
		return -1;
	}
	
	SDL_WM_SetCaption(caption, 0);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	glClear(GL_COLOR_BUFFER_BIT);
	SDL_GL_SwapBuffers();
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_SHARED_TEXTURE_PALETTE_EXT);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	return 0;
#else
	// to prevent compiler complaining about unused variables
	(void)width; (void)height; (void)bits_per_pixel; (void)caption;
	return -1;
#endif
}

int opengl_renderer_quit(void)
{
#ifdef USE_OPENGL
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	
	return 0;
#else
	return -1;
#endif
}

void SurfaceFormatInfo(SDL_PixelFormat *f)
{
	fprintf(stderr, "palette %p\n", f->palette);
	fprintf(stderr, "BitsPerPixel %d\n", f->BitsPerPixel);
	fprintf(stderr, "BytesPerPixel %d\n", f->BytesPerPixel);
	fprintf(stderr, "Rmask %x Gmask %x Bmask %x Amask %x\n", f->Rmask, f->Gmask, f->Bmask, f->Amask);
	fprintf(stderr, "Rshift %d Gshift %d Bshift %d Ashift %d\n", f->Rshift, f->Gshift, f->Bshift, f->Ashift);
	fprintf(stderr, "Rloss %d Gloss %d Bloss %d Aloss %d\n", f->Rloss, f->Gloss, f->Bloss, f->Aloss);
	fprintf(stderr, "colorkey %x\n", f->colorkey);
	fprintf(stderr, "alpha %x\n", f->alpha);
}

int opengl_renderer_load_texture(SDL_RWops *data, Image *im)
{
#ifdef USE_OPENGL
	SDL_Surface *pic;
	
	if ((pic = IMG_Load_RW(data, 0)) == 0)
	{
		fprintf(stderr, "opengl_renderer_load_texture: %s\n", IMG_GetError());
		return -1;
	}
	
	im->width  = pic->w;
	im->height = pic->h;
	
	// palette is not needed, its read from another place
	// *palette = malloc(256*4);
	// memcpy(*palette, pic->format->palette->colors, 256*4);
	
	// image data is stored row-by-row
	// and each row is padded to length of 4 bytes
	// (so 110x110 is actually 112x110)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	
	glGenTextures(1, &im->tex);
	glBindTexture(GL_TEXTURE_2D, im->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_COLOR_INDEX8_EXT, pic->w, pic->h, 0,
					GL_COLOR_INDEX, GL_UNSIGNED_BYTE, pic->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	SDL_FreeSurface(pic);
	
	return 0;
#else
	// to prevent compiler complaining about unused variables
	(void)data; (void)im;
	return -1;
#endif
}

int opengl_renderer_load_texture_without_palette(SDL_RWops *data, Image *im)
{
#ifdef USE_OPENGL
	SDL_Surface *pic;
	SDL_Surface *adapted;
	SDL_PixelFormat fmt;
	
	if ((pic = IMG_Load_RW(data, 0)) == 0)
	{
		fprintf(stderr, "opengl_renderer_load_texture: %s\n", IMG_GetError());
		return -1;
	}
	
	im->width  = pic->w;
	im->height = pic->h;
	
	// SDL surface is stored as BGR, but OpenGL needs RGB
	// so we change that here
	fmt.BitsPerPixel = 32;
	fmt.BytesPerPixel = 4;
	fmt.Rmask = 0x000000ff; fmt.Rshift = 0;
	fmt.Gmask = 0x0000ff00; fmt.Gshift = 8;
	fmt.Bmask = 0x00ff0000; fmt.Bshift = 16;
	fmt.Amask = 0xff000000; fmt.Ashift = 24;
	fmt.alpha = 0xff;
	fmt.colorkey = 0;
	fmt.palette = 0;
	adapted = SDL_ConvertSurface(pic, &fmt, SDL_SWSURFACE|SDL_HWSURFACE);
	//SurfaceFormatInfo(pic->format);
	//SurfaceFormatInfo(&fmt);
	//SurfaceFormatInfo(adapted->format);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	
	glGenTextures(1, &im->tex);
	glBindTexture(GL_TEXTURE_2D, im->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pic->w, pic->h, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, adapted->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	SDL_FreeSurface(pic);
	SDL_FreeSurface(adapted);
	
	return 0;
#else
	// to prevent compiler complaining about unused variables
	(void)data; (void)im;
	return -1;
#endif
}

int opengl_renderer_free_texture(Image *im)
{
#ifdef USE_OPENGL
	glDeleteTextures(1, &im->tex);
	
	return 0;
#else
	// to prevent compiler complaining about unused variables
	(void)im;
	return -1;
#endif
}

int opengl_renderer_draw(Image *im, void *pal, int x, int y)
{
#ifdef USE_OPENGL
	if (pal)
	{
		// set alpha on colorkey index
		if (im->colorkey_index >= 0 && im->colorkey_index <= 255)
			((unsigned char*)pal+4*im->colorkey_index)[3] = 0;
		
		glColorTable(GL_SHARED_TEXTURE_PALETTE_EXT, GL_RGBA, 256, GL_RGBA,
		             GL_UNSIGNED_BYTE, pal);
	}
	
	glTranslatef(x, y, 0);
	glBindTexture(GL_TEXTURE_2D, im->tex);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 1); glVertex2f(        0,          0);
		glTexCoord2f(1, 1); glVertex2f(im->width,          0);
		glTexCoord2f(1, 0); glVertex2f(im->width, im->height);
		glTexCoord2f(0, 0); glVertex2f(        0, im->height);
	glEnd();
	glTranslatef(-x, -y, 0);
	
	if (pal)
		if (im->colorkey_index >= 0 && im->colorkey_index <= 255)
			((unsigned char*)pal+4*im->colorkey_index)[3] = 255;
	
	return 0;
#else
	// to prevent compiler complaining about unused variables
	(void)im; (void)pal; (void)x; (void)y;
	return -1;
#endif
}

int opengl_renderer_drawing_begin()
{
#ifdef USE_OPENGL
	//glClear(GL_COLOR_BUFFER_BIT);
	return 0;
#else
	return -1;
#endif
}

int opengl_renderer_drawing_end()
{
#ifdef USE_OPENGL
	SDL_GL_SwapBuffers();
	
	return 0;
#else
	return -1;
#endif
}

int opengl_renderer_clear_screen()
{
#ifdef USE_OPENGL
	glClear(GL_COLOR_BUFFER_BIT);
	//SDL_GL_SwapBuffers();
	
	return 0;
#else
	return -1;
#endif
}
