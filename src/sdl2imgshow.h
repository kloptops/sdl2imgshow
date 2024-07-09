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

#define UNUSED(x) (void)(x)


#define ASSIGN_RECT(to_var, from_var) \
    { \
        to_var.x = from_var.x; \
        to_var.y = from_var.y; \
        to_var.w = from_var.w; \
        to_var.h = from_var.h; \
    }

#define ASSIGN_COLOR(to_var, from_var) \
    { \
        to_var.r = from_var.r; \
        to_var.g = from_var.g; \
        to_var.b = from_var.b; \
        to_var.a = from_var.a; \
    }

#define ASSIGN_POINT(to_var, from_var) \
    {\
        to_var.x = from_var.x; \
        to_var.y = from_var.y; \
    }


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
    bool         duplicate;
} Image_Object;


typedef struct _Option_List
{
    struct _Option_List *next;
    struct _Option_List *prev;
    char *id;
    Image_Object *image_object;
} Option_List;


typedef void (*ini_callback)(void *state, const char *key, const char *value);

// Globals
extern Image_Object *global_image;
extern Image_Object *root_image;
extern Option_List *root_option;

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
Image_Object *image_copy_stack();
Image_Object *image_global_duplicate();

void image_init();
void image_quit();

int sdl_do_init();
void sdl_do_quit();

void var_set_parse(const char *text, bool var_sub);
void ini_parse(void *state, const char *key, const char *value);
void option_parse(void *state, const char *key, const char *value);
int ini_read(const char *filename, ini_callback callback, void *state);

bool load_font(const char *fontFile);
void font_size(int fontSize);
bool load_image(const char *imageFile);
bool render_text(const char *text);

void *ez_malloc(size_t size);
char *ez_strcatn(char *str1, const char *str2, size_t str2_len);

int get_positon(const char *positon);
int get_image_size(const char *size);
int get_text_align(const char *text_align);

void init_vars();
void quit_vars();
void set_var(const char *name, const char *value);
const char *get_var(const char *name);

char *sub_vars(const char *input);

void calculate_texture_rect(SDL_Texture *imageTexture, SDL_Rect *textureRect, int position);
void calculate_texture_size(SDL_Texture *imageTexture, SDL_Rect *textureRect, int size);

int strncasecmp(const char *s1, const char *s2, size_t n);
int strcasecmp(const char *s1, const char *s2);
bool strcasestartswith(const char *str, const char *prefix);
bool strstartswith(const char *str, const char *prefix);
bool strcaseendswith(const char *str, const char *suffix);
bool strendswith(const char *str, const char *suffix);

bool file_exists(const char *filename);

#endif /* __SDL2IMGSHOW_H__ */
