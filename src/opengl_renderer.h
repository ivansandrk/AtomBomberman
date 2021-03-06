/**
  @file     opengl_renderer.h
  @author   ivansandrk
  @date     Aug 2011
  @version  $Revision: 0.1$
  @brief    OpenGL renderer for Atomic Bomberman clone
  
  long description here
*/

// | - what is this?
// v
/*
	$Id: opengl_renderer.h,v 0.1 2011-08-15 19:55:00 ivansandrk Exp $
	$Revision: 0.1 $
*/


#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H


// included here because of SDL_RWops and GLuint
#include "SDL.h"
#include "SDL_opengl.h"
#include "graphics.h"

/**
  @brief  Initialise OpenGL renderer
  @param  width Screen width, in pixels
  @param  height Screen height, in pixels
  @param  bits_per_pixel Screen bits per pixel
  @param  caption Window caption
  @return 0 on success, -1 on failure
  
  On failure prints the error message to stderr. It is possible to use a
  fallback display renderer.
*/
int opengl_renderer_init(int width, int height, int bits_per_pixel, const char caption[]);

/**
  @brief  Quit OpenGL renderer
  @return 0 on success, -1 on failure
*/
int opengl_renderer_quit();

/**
  @brief  Load an image as an OpenGL texture (possibly in video memory)
  @param  data Image data
  @param  im Image structure to hold the data
  @return 0 on success, -1 on failure
  
  Image has a palette, which is loaded elsewhere.
  (color indexed image)
*/
int opengl_renderer_load_texture(SDL_RWops *data, Image *im);

/**
  @brief  Load an image as an OpenGL texture (possibly in video memory)
  @param  data Image data
  @param  im Image structure to hold the data
  @return 0 on success, -1 on failure
  
  Image has no palette, it is 'raw'.
*/
int opengl_renderer_load_texture_without_palette(SDL_RWops *data, Image *im);

/**
  @brief  Free a previously loaded OpenGL texture
  @param  im Image structure associated with the texture
  @return 0 on success, -1 on failure
*/
int opengl_renderer_free_texture(Image *im);

/**
  @brief  Draw a texture to the screen
  @param  im Image structure to draw
  @param  pal Palette used to draw the texture
  @param  x Drawing location
  @param  y Drawing location
  @return 0 on success, -1 on failure
*/
int opengl_renderer_draw(Image *im, void *pal, int x, int y);

/**
  @brief  Begin drawing
  @return 0 on success, -1 on failure
  
  Currently does nothing; may change to a call to opengl_renderer_clear_screen().
*/
int opengl_renderer_drawing_begin();

/**
  @brief  End drawing
  @return 0 on success, -1 on failure
  
  This just calls SDL_GL_SwapBuffers().
*/
int opengl_renderer_drawing_end();

/**
  @brief  Clear the screen
  @return 0 on success, -1 on failure
*/
int opengl_renderer_clear_screen();


#endif /* OPENGL_RENDERER_H */

