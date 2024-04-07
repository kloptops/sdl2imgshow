#ifndef __SDL2IMGSHOW_H__
#define __SDL2IMGSHOW_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// Types
enum
{
    POS_TOPLEFT,
    POS_TOPCENTER,
    POS_TOPRIGHT,
    POS_MIDLEFT,
    POS_CENTER,
    POS_MIDRIGHT,
    POS_BOTTOMLEFT,
    POS_BOTTOMCENTER,
    POS_BOTTOMRIGHT,
};

enum
{
    SIZE_FIT,
    SIZE_VERTICAL,
    SIZE_HORIZONTAL,
    SIZE_ORIGINAL,
    SIZE_STRETCH,
};

extern int imageSize;


enum
{
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT,
};

typedef struct _Image_Object
{
    struct _Image_Object *next;
    SDL_Texture *imageTexture;
    SDL_Rect     imageRect;
    SDL_Color    drawColor;
} Image_Object;


typedef void (*ini_callback)(void *state, const char *key, const char *value);

// Globals
extern Image_Object *root_image;

extern int screenWidth;
extern int screenHeight;
extern int imageSize;
extern int imagePosition;
extern int textPosition;
extern int textAlignment;
extern int fontSize;
extern bool dropShadow;
extern bool wantQuit;

extern TTF_Font* globalFont;
extern SDL_Rect  globalMargins;
extern SDL_Color textColor;
extern SDL_Color dropShadowColor;
extern SDL_Point dropShadowOffset;

// Functions
void *ez_malloc(size_t size);

Image_Object *image_create();
void image_quit();

int sdl_do_init();
void sdl_do_quit();

void ini_parse(void *state, const char *key, const char *value);
void ini_read(const char *filename, ini_callback callback, void *state);

int load_font(const char *fontFile);
void font_size(int fontSize);
int load_image(const char *imageFile);
int render_text(const char *text);

void *ez_malloc(size_t size);
int get_positon(const char *positon);
int get_image_size(const char *size);
int get_text_align(const char *text_align);
char *sub_env_vars(const char *input);

void calculate_texture_rect(SDL_Texture *imageTexture, SDL_Rect *textureRect, int position);
void calculate_texture_size(SDL_Texture *imageTexture, SDL_Rect *textureRect, int size);

int strncasecmp(const char *s1, const char *s2, size_t n);
int strcasecmp(const char *s1, const char *s2);
bool strcasestartswith(const char *str, const char *prefix);
bool strstartswith(const char *str, const char *prefix);
bool strcaseendswith(const char *str, const char *suffix);
bool strendswith(const char *str, const char *suffix);

#endif /* __SDL2IMGSHOW_H__ */
