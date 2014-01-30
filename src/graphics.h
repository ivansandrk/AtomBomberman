/**
  @file    graphics.h
  @author  johndoevodka
  @date    Aug 2011
  @version 0.1
  @brief   Middle layer that seperates game code and rendering code; it abstracts the rendering code.
  
  Uses SDL renderer as the backbone. OpenGL renderer currently isn't supported,
  at some point in the past it used to be.
*/

#ifndef GRAPHICS_H
#define GRAPHICS_H


// included here because of SDL_RWops, GLuint
#include "SDL.h"
#ifdef USE_OPENGL
#include "SDL_opengl.h"
#endif


typedef struct _Image {
	// number associated with the loaded texture
	union {
#ifdef USE_OPENGL
		GLuint tex;
#endif
		SDL_Surface *surf;
	};
	
	int width;
	int height;
	
	// hotspots are used for drawing sprites
	// they tell the spot where the sprite is drawn from
	int hotspot_x;
	int hotspot_y;
	
	// this variable tells us which pallete entry is the colorkey one
	int colorkey_index;
} Image;


typedef struct _Frame {
	// image that is part of animation
	Image *im;
	
	// these are needed for adjustments of images
	int x;
	int y;
} Frame;


typedef struct _Anim {
	// how many frames in this animation
	int n_frames;
	
	// all the frames
	Frame *frames;
	
	// name of animation
	char *name;
} Anim;


typedef struct _ANI {
	// info about loaded images
	int n_images;
	Image **im;
	
	// info about different animations that are part of the ANI file
	int n_anim;
	Anim **anim;
} ANI;



/**
  @brief  Initialise graphics subsystem
  @return 0 on success, -1 on failure
*/
int graphics_init();

/**
  @brief  Quit graphics subsystem
  @return 0 on success, -1 on failure
*/
int graphics_quit();

/**
  @brief  Load an image from SDL_RWops into memory
  @param  file Image file location
  @param  hotspot_x Image hotspot x
  @param  hotspot_y Image hotspot y
  @param  colorkey_index Image colorkey palette index
  @return Image struct containing data needed for drawing or 0 on failure
  
  Image needs a palette for drawing.
*/
Image* load_image(char *file, int hotspot_x, int hotspot_y, int colorkey_index);

/**
  @brief  Load an image from SDL_RWops into memory
  @param  file Image file location
  @return Image struct containing data needed for drawing or 0 on failure
  
  Image doesnt need a palette, it is 'raw'.
*/
Image* load_image_without_palette(char *file);

/**
  @brief  Free a previously loaded image
  @param  im Image struct
  @return 0 on success, -1 if im == 0
*/
int free_image(Image *im);

/**
  @brief  Draw an image to the screen
  @param  im Image struct
  @param  color Image color (basically specifies the palette)
  @param  x Drawing location
  @param  y Drawing location
  @return 0 on success, -1 on failure
*/
int draw(Image *im, int color, int x, int y);

/**
  @brief  Draw an image to the screen, to tile location (row, col)
  @param  im Image struct
  @param  color Image color (basically specifies the palette)
  @param  row Drawing location, row
  @param  col Drawing location, column
  @return 0 on success, -1 on failure
*/
int draw_tiled(Image *im, int color, int row, int col);

/**
  @brief  Draw an image to the screen, inside the play zone
  @param  im Image struct
  @param  color Image color (basically specifies the palette)
  @param  x Drawing location, inside the play zone
  @param  y Drawing location, inside the play zone
  @return 0 on success, -1 on failure
*/
int draw_llc(Image *im, int color, int x, int y);

/**
  @brief  Load all (10) palettes
  @return 0 on success, -1 on failure
  
  All palettes are stored in one file, sequentially, as 10 x 256 x 3 Bytes (RGB).
*/
int load_palettes();

/**
  @brief  Free all (10) palettes
  @return 0 on success, -1 on failure
*/
int free_palettes();

/**
  @brief  Begin drawing
  @return 0 on success, -1 on failure
*/
int drawing_begin();

/**
  @brief  End drawing
  @return 0 on success, -1 on failure
*/
int drawing_end();

/**
  @brief  Clear the screen
  @return 0 on success, -1 on failure
*/
int clear_screen();

/**
  @brief  Loads an ANI file (JDV uncompressed version) into memory
  @param  path Path to animation directory
  @return ANI structure that holds all the data about the animation (or several animations)
*/
ANI* load_anim(const char *path);

/**
  @brief  Free an ANI struct
  @param  ANI struct
  @return 0 on success, -1 on failure
*/
int free_ani(ANI *ani);

int print(int x, int y, char *str);

/** *************************************** hackity-hack-hack *****************************/
int draw_square(int x, int y, int size, int color);
int draw_vline(int x, int y0, int y1, int color);
int draw_hline(int x0, int x1, int y, int color);
int draw_grid();
int draw_bounding_box(int x, int y, int width, int height, int color);
/** *************************************** hackity-hack-hack *****************************/


#endif /* GRAPHICS_H */

