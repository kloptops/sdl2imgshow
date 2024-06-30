// SPDX-License-Identifier: MIT

#include "sdl2imgshow.h"

Image_Object *root_image = NULL;

int screenWidth   = 640;
int screenHeight  = 480;
int imageSize     = SIZE_VERTICAL;
int imagePosition = POS_MIDLEFT;
int textPosition  = POS_CENTER;
int textAlignment = ALIGN_LEFT;
int fontSize      = 32;

bool dropShadow   = false;

bool wantQuit     = false;   // quit immediately
bool waitQuit     = false;   // wait for a button press to quit
bool keypressQuit = false;    // wait until 30 presses have been done to quit

bool wantQuiet    = false;    // wait quietly

char *globalFontName       = NULL;
TTF_Font *globalFont       = NULL;
SDL_Rect  globalMargins    = {0, 0, 0, 0};
SDL_Color textColor        = {0, 0, 0, 255};
SDL_Color dropShadowColor  = {125, 125, 125, 255};
SDL_Point dropShadowOffset = {10, 10};

SDL_Window   *window   = NULL;
SDL_Renderer *renderer = NULL;

char processWatchCmd[1024] = "";
bool processWatch = false;

void print_usage()
{
    printf("Usage: sdl2imgshow [-z <config_file.ini>] [-i <image_file>] [-p text_positon] [-f <font_file>] [-c <R,G,B>] [-d <R,G,B>] [-o x,y] [-s <font_size>] [-t <text>] [-q] [-b <process>]\n");
}


int main(int argc, char *argv[])
{
    if (sdl_do_init() != 0)
    {
        sdl_do_quit();
        return 255;
    }

    // Get screen resolution
    SDL_DisplayMode dm;
    if (SDL_GetCurrentDisplayMode(0, &dm) != 0)
    {
        printf("SDL_GetCurrentDisplayMode Error: %s\n", SDL_GetError());
        sdl_do_quit();
        return 1;
    }

    screenWidth  = dm.w;
    screenHeight = dm.h;

    // Create window
    window = SDL_CreateWindow("SDL2 Image Show",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screenWidth, screenHeight, SDL_WINDOW_FULLSCREEN);

    if (window == NULL)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        sdl_do_quit();
        return 1;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        SDL_DestroyWindow(window);
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        sdl_do_quit();
        return 1;
    }

    int opt;
    bool finished = false;

    while (!finished && (opt = getopt(argc, argv, "z:i:f:t:c:s:d:o:a:S:q:Dp:k:w:W:b:")) != -1)
    {
        switch (opt)
        {
        case 'z':
            ini_read(optarg, &ini_parse, NULL);
            break;

        case 'i':
            ini_parse(NULL, "image", optarg);
            break;

        case 'a':
            ini_parse(NULL, "text_align", optarg);
            break;

        case 'f':
            ini_parse(NULL, "font", optarg);
            break;

        case 't':
            ini_parse(NULL, "text", optarg);
            break;

        case 'c':
            ini_parse(NULL, "text_color", optarg);
            break;

        case 'P':
            ini_parse(NULL, "image_position", optarg);
            break;

        case 'S':
            ini_parse(NULL, "image_stretch", optarg);
            break;

        case 's':
            ini_parse(NULL, "font_size", optarg);
            break;

        case 'p':
            ini_parse(NULL, "text_position", optarg);
            break;

        case 'd':
            ini_parse(NULL, "shadow_color", optarg);
            break;

        case 'o':
            ini_parse(NULL, "shadow_offset", optarg);
            break;

        case 'D':
            ini_parse(NULL, "shadow", "n");
            break;

        case 'q':
            ini_parse(NULL, "quit", optarg);
            break;

        case 'k':
            ini_parse(NULL, "keypress_quit", optarg);
            break;

        case 'w':
            ini_parse(NULL, "wait_quit", optarg);
            break;

        case 'W':
            ini_parse(NULL, "quiet", optarg);
            break;

        case 'b':
            snprintf(processWatchCmd, sizeof(processWatch), "pgrep '%s'", optarg);
            processWatch = true;
            break;

        default: /* '?' */
            finished = true;
            break;
        }
    }

    // Check if required arguments are provided
    if (finished == true)
    {
        print_usage();

        image_quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        sdl_do_quit();
        return EXIT_FAILURE;
    }

    SDL_Event event;
    int quit = 0;
    bool doneRender = false;
    int keypressQuitCount = 0;

    // Wait for quit event
    while (!quit)
    {
        while (SDL_PollEvent(&event) && !quit)
        {
            switch (event.type)
            {
            case SDL_CONTROLLERBUTTONDOWN:
                keypressQuitCount += 1;

                if (keypressQuit && keypressQuitCount > 30)
                    quit = 1;
                break;

            case SDL_CONTROLLERBUTTONUP:
                if (waitQuit)
                    quit = 1;

                break;

            case SDL_CONTROLLERDEVICEADDED:
                {
                    SDL_GameController* controller = SDL_GameControllerOpen(event.cdevice.which);
                }
                break;
            case SDL_CONTROLLERDEVICEREMOVED:
                {
                    SDL_GameController* controller = SDL_GameControllerFromInstanceID(event.cdevice.which);
                    if (controller)
                    {
                        SDL_GameControllerClose(controller);
                    }
                }
                break;

            case SDL_QUIT:
                quit = 1;
                break;
            }
        }

        Image_Object *current = root_image;

        if (!doneRender)
        {
            printf("loop\n");
            // Clear screen
            // SDL_RenderClear(renderer);

            // Render Textures
            while (current != NULL)
            {
                if (current->imageTexture != NULL)
                {
                    SDL_SetTextureColorMod(
                        current->imageTexture,
                        current->drawColor.r, current->drawColor.g, current->drawColor.b);

                    SDL_RenderCopy(renderer, current->imageTexture, NULL, &current->imageRect);
                }

                current = current->next;
            }

            // Update screen
            SDL_RenderPresent(renderer);
            doneRender = wantQuiet;
        }

        // 10 frames a second is fine.
        SDL_Delay(100);

        if (wantQuit == true)
            break;

        if (processWatch)
        {
            if (system(processWatchCmd) == 0)
                break;
        }
    }

    // Clean up
    image_quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    sdl_do_quit();

    return 0;
}


static int sdl_status = 0;

int sdl_do_init()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    sdl_status += 1;

    // Initialize SDL_image
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        return 1;
    }

    sdl_status += 1;

    // Initialize SDL_ttf
    if (TTF_Init() != 0)
    {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        return 1;
    }

    sdl_status += 1;

    const char* db_file = SDL_getenv("SDL_GAMECONTROLLERCONFIG_FILE");
    if (db_file)
    {
        SDL_GameControllerAddMappingsFromFile(db_file);
    }

    return 0;
}

void sdl_do_quit()
{
    if (sdl_status > 2)
    {
        if (globalFont != NULL)
        {
            TTF_CloseFont(globalFont);
            globalFont = NULL;
        }

        TTF_Quit();
    }

    if (sdl_status > 1)
    {
        IMG_Quit();
    }

    if (sdl_status > 0)
    {
        SDL_Quit();
    }

    sdl_status = 0;
}


bool bool_parse(const char *value, bool defaultBool)
{
    if (strcasecmp(value, "y") == 0 || strcasecmp(value, "true") == 0)
        return true;

    else if (strcasecmp(value, "n") == 0 || strcasecmp(value, "false") == 0)
        return false;

    return defaultBool;
}


void ini_parse(void *state, const char *key, const char *value)
{   // Callback for INI parsing, also used in getopt.
    if (strcasecmp(key, "image") == 0)
    {
        load_image(value);
    }
    else if (strcasecmp(key, "image_position") == 0)
    {
        imagePosition = get_positon(value);
    }
    else if (strcasecmp(key, "image_stretch") == 0)
    {
        imageSize = get_image_size(value);
    }
    else if (strcasecmp(key, "text_position") == 0)
    {
        textPosition = get_positon(value);
    }
    else if (strcasecmp(key, "screen_margin") == 0)
    {
        sscanf(value, "%d,%d,%d,%d", &globalMargins.x, &globalMargins.y, &globalMargins.w, &globalMargins.h);
    }
    else if (strcasecmp(key, "font") == 0)
    {
        load_font(value);
    }
    else if (strcasecmp(key, "font_size") == 0)
    {
        font_size(atoi(value));
    }
    else if (strcasecmp(key, "text") == 0)
    {
        render_text(value);
    }
    else if (strcasecmp(key, "text_color") == 0)
    {
        sscanf(value, "%d,%d,%d", &textColor.r, &textColor.g, &textColor.b);
    }
    else if (strcasecmp(key, "shadow_color") == 0)
    {
        sscanf(value, "%d,%d,%d", &dropShadowColor.r, &dropShadowColor.g, &dropShadowColor.b);
        dropShadow=true;
    }
    else if (strcasecmp(key, "shadow_offset") == 0)
    {
        sscanf(value, "%d,%d", &dropShadowOffset.x, &dropShadowOffset.y);
        dropShadow=true;
    }
    else if (strcasecmp(key, "shadow") == 0)
    {
        dropShadow = bool_parse(value, false);
    }
    else if (strcasecmp(key, "quiet") == 0)
    {
        wantQuiet = bool_parse(value, false);
    }
    else if (strcasecmp(key, "quit") == 0)
    {
        wantQuit = bool_parse(value, false);
    }
    else if (strcasecmp(key, "wait_quit") == 0)
    {
        waitQuit = bool_parse(value, false);
    }
    else if (strcasecmp(key, "keypress_quit") == 0)
    {
        keypressQuit = bool_parse(value, false);
    }
    else
    {
        fprintf(stderr, "Unknown INI: %s = %s\n", key, value);
    }
}


#define MAX_RENDER_LINES 10
SDL_Surface* render_text_wrapped(const char* text)
{
    // Split the text into lines
    char *token;
    char *rest = strdup(text);
    char *lines[MAX_RENDER_LINES];
    int numLines = 0;

    while ((token = strtok_r(rest, "||", &rest)) && numLines < MAX_RENDER_LINES)
    {
        lines[numLines++] = token;
    }

    // Calculate the widest surface
    int maxWidth = 0;
    for (int i = 0; i < numLines; ++i)
    {
        int width, height;
        TTF_SizeText(globalFont, lines[i], &width, &height);

        if (width > maxWidth)
        {
            maxWidth = width;
        }
    }

    // Create a surface to render the text
    SDL_Surface* renderedSurface = SDL_CreateRGBSurfaceWithFormat(0, maxWidth, numLines * TTF_FontLineSkip(globalFont), 32, SDL_PIXELFORMAT_RGBA8888);
    if (!renderedSurface)
    {
        printf("Unable to create surface for rendering text! SDL Error: %s\n", SDL_GetError());
        return NULL;
    }

    // Fill the surface with a transparent background
    SDL_FillRect(renderedSurface, NULL, SDL_MapRGBA(renderedSurface->format, 0, 0, 0, 0));

    // Render each line of text onto the surface with alignment
    int yOffset = 0;
    printf("render_text_wrapped:\n");
    for (int i = 0; i < numLines; ++i)
    {
        printf("- %s\n", lines[i]);
        SDL_Surface* tempSurface = TTF_RenderText_Blended_Wrapped(globalFont, lines[i], (SDL_Color){255, 255, 255, 255}, maxWidth);
        if (!tempSurface)
        {
            printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
            continue;
        }

        int xOffset = 0;
        if (textAlignment == ALIGN_CENTER)
        {   // Center alignment
            xOffset = (maxWidth - tempSurface->w) / 2;
        }
        else if (textAlignment == ALIGN_RIGHT)
        {   // Right alignment
            xOffset = maxWidth - tempSurface->w;
        }

        SDL_Rect destRect = {xOffset, yOffset, tempSurface->w, tempSurface->h};
        SDL_BlitSurface(tempSurface, NULL, renderedSurface, &destRect);
        SDL_FreeSurface(tempSurface);

        yOffset += TTF_FontLineSkip(globalFont);
    }

    // Free memory
    // free(rest);

    return renderedSurface;
}


int load_image(const char *imageFile)
{
    // Load image
    char *imageRef = sub_env_vars(imageFile);

    if (imageRef == NULL)
        return 1;

    SDL_Surface *imageSurface = IMG_Load(imageRef);
    free(imageRef);

    if (imageSurface == NULL)
    {
        fprintf(stderr, "IMG: Couldn't load %s: %s\n", imageRef, IMG_GetError());
        return 1;
    }

    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    if (imageSurface == NULL)
    {
        return 1;
    }

    Image_Object *image = image_create();

    image->imageTexture = imageTexture;

    calculate_texture_size(image->imageTexture, &image->imageRect, imageSize);
    calculate_texture_rect(image->imageTexture, &image->imageRect, imagePosition);

    return 0;
}

void font_size(int size)
{
    if (size < 1)
        return;

    fontSize = size;

    if (globalFont != NULL)
    {
        load_font(globalFontName);
    }
}


int load_font(const char *fontFile)
{
    int scaleSize = (int)(float)((screenHeight / 480.0f) * (float)fontSize);

    char *fontRef = sub_env_vars(fontFile);
    if (fontRef == NULL)
    {
        return 1;
    }

    TTF_Font *oldFont = globalFont;

    globalFont = TTF_OpenFont(fontRef, scaleSize);
    if (globalFont == NULL)
    {   // Restore old font.
        globalFont = oldFont;

        fprintf(stderr, "TTF: Couldn't load %s: %s\n", fontRef, TTF_GetError());
        return 1;
    }

    if (oldFont != NULL)
    {
        free(globalFontName);
        TTF_CloseFont(oldFont);
    }

    globalFontName = fontRef;

    return 0;
}


int render_text(const char *text)
{
    char *textRef = sub_env_vars(text);
    if (textRef == NULL)
        return 1;        

    SDL_Surface *imageSurface = render_text_wrapped(textRef);
    if (imageSurface == NULL)
    {
        fprintf(stderr, "TTF: Couldn't render \"%s\" -> \"%s\": %s\n", text, textRef, IMG_GetError());
        free(textRef);
        return 1;
    }

    free(textRef);

    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    if (imageSurface == NULL)
    {
        return 1;
    }

    Image_Object *dropImage = NULL;
    if (dropShadow)
        dropImage = image_create();

    Image_Object *image = image_create();

    image->imageTexture = imageTexture;
    image->drawColor.r = textColor.r;
    image->drawColor.g = textColor.g;
    image->drawColor.b = textColor.b;

    calculate_texture_size(image->imageTexture, &image->imageRect, SIZE_ORIGINAL);
    calculate_texture_rect(image->imageTexture, &image->imageRect, textPosition);

    if (dropShadow)
    {
        dropImage->imageTexture = imageTexture;
        dropImage->drawColor.r = dropShadowColor.r;
        dropImage->drawColor.g = dropShadowColor.g;
        dropImage->drawColor.b = dropShadowColor.b;

        dropImage->imageRect.x = image->imageRect.x + dropShadowOffset.x;
        dropImage->imageRect.y = image->imageRect.y + dropShadowOffset.y;
        dropImage->imageRect.w = image->imageRect.w;
        dropImage->imageRect.h = image->imageRect.h;
    }

    return 0;
}


Image_Object *image_create()
{
    Image_Object *image = (Image_Object *)ez_malloc(sizeof(Image_Object));

    image->drawColor.r = 255;
    image->drawColor.g = 255;
    image->drawColor.b = 255;
    image->drawColor.a = 255;

    if (root_image == NULL)
    {
        root_image = image;
    }
    else
    {
        Image_Object *current = root_image;

        while (current->next != NULL)
            current = current->next;

        current->next = image;
    }

    return image;
}

void image_quit()
{
    Image_Object *current = root_image;
    Image_Object *next = NULL;

    while (current != NULL)
    {
        next = current->next;

        if (current->imageTexture != NULL)
            SDL_DestroyTexture(current->imageTexture);

        free(current);

        current = next;
    }

    root_image = NULL;
}
