// SPDX-License-Identifier: MIT

#include "sdl2imgshow.h"

Image_Object *global_image = NULL;
Image_Object *root_image = NULL;
Option_List *root_option = NULL;

int screenWidth   = 640;
int screenHeight  = 480;
int imageSize     = SIZE_VERTICAL;
int imagePosition = POS_MIDLEFT;
int textPosition  = POS_CENTER;
int textAlignment = ALIGN_LEFT;
int fontSize      = 32;

bool dropShadow   = false;
bool imageFallback = false;
bool fontFallback  = false;

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

const char *displayTemplate = NULL;

char processWatchCmd[1024] = "";
bool processWatch = false;


void print_usage()
{
    fprintf(stderr, "Usage: sdl2imgshow [-z <config_file.ini>] [-i <image_file>] [-p text_positon] [-f <font_file>] [-c <R,G,B>] [-d <R,G,B>] [-o x,y] [-s <font_size>] [-t <text>] [-q] [-b <process>]\n");
}


int main(int argc, char *argv[])
{
    char tempBuff[40];

    if (sdl_do_init() != 0)
    {
        sdl_do_quit();
        return 255;
    }

    // Get screen resolution
    SDL_DisplayMode dm;
    if (SDL_GetCurrentDisplayMode(0, &dm) != 0)
    {
        fprintf(stderr, "SDL_GetCurrentDisplayMode Error: %s\n", SDL_GetError());
        sdl_do_quit();
        return 1;
    }

    screenWidth  = dm.w;
    screenHeight = dm.h;

    snprintf(tempBuff, sizeof(tempBuff), "%d", screenWidth);
    set_var("width", tempBuff);

    snprintf(tempBuff, sizeof(tempBuff), "%d", screenHeight);
    set_var("height", tempBuff);

    // Create window
    window = SDL_CreateWindow("SDL2 Image Show",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screenWidth, screenHeight, SDL_WINDOW_FULLSCREEN);

    if (window == NULL)
    {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        sdl_do_quit();
        return 1;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        SDL_DestroyWindow(window);
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        sdl_do_quit();
        return 1;
    }

    int opt=0;
    bool finished = false;
    bool option_select_mode=false;
    const char *option_select_file=NULL;
    const char *default_select=NULL;

    while (!finished && (opt = getopt(argc, argv, "z:i:f:t:c:s:d:o:a:S:q:Dp:k:w:W:b:T:F:G:x:X:")) != -1)
    {
        switch (opt)
        {
        case 'z':
            // -z <config_file>: config file
            ini_read(optarg, &ini_parse, NULL);
            break;

        case 'T':
            // -T <display_template>: display template for game select mode
            displayTemplate=optarg;
            break;

        case 'F':
            // -F <game_id>: default game selected
            default_select=optarg;
            break;

        case 'G':
            // -G <option_file.ini>: option files file.
            option_select_mode=true;
            option_select_file=optarg;
            break;

        case 'i':
            // -i <image_file>: load an image and add it to the stack
            ini_parse(NULL, "image", optarg);
            break;

        case 'a':
            // -a <text_alignment>: set text alignment
            ini_parse(NULL, "text_align", optarg);
            break;

        case 'f':
            // -f <font_file>: load font.
            ini_parse(NULL, "font", optarg);
            break;

        case 't':
            // -t <text>: render text to the display.
            ini_parse(NULL, "text", optarg);
            break;

        case 'c':
            // -c <colour>: set text_color
            ini_parse(NULL, "text_color", optarg);
            break;

        case 'P':
            // -P <image_positon>: set image_position mode
            ini_parse(NULL, "image_position", optarg);
            break;

        case 'S':
            // -S <image_stretch>: set image_stretch mode
            ini_parse(NULL, "image_stretch", optarg);
            break;

        case 's':
            // -s <font_size>: set the current font size.
            ini_parse(NULL, "font_size", optarg);
            break;

        case 'p':
            // -p <text_position>: set text_positon.
            ini_parse(NULL, "text_position", optarg);
            break;

        case 'd':
            // -d <shadow_color>: sets drop shadow colour, enables drop shadows.
            ini_parse(NULL, "shadow_color", optarg);
            break;

        case 'o':
            // -o <shadow_offset>: sets shadow_offset for the drop shadow.
            ini_parse(NULL, "shadow_offset", optarg);
            break;

        case 'D':
            // -D: disable drop shadow.
            ini_parse(NULL, "shadow", "n");
            break;

        case 'q':
            // -q: quit immediately mode.
            ini_parse(NULL, "quit", optarg);
            break;

        case 'k':
            // -k: keypress quit mode.
            ini_parse(NULL, "keypress_quit", optarg);
            break;

        case 'w':
            // -W: wait quit mode.
            ini_parse(NULL, "wait_quit", optarg);
            break;

        case 'W':
            // -W: quiet mode.
            ini_parse(NULL, "quiet", optarg);
            break;

        case 'b':
            // -b <process_name>: watch for process_name, quit if it is running.
            snprintf(processWatchCmd, sizeof(processWatch), "pgrep '%s'", optarg);
            processWatch = true;
            break;

        case 'x':
            // -x <key=value>: set a variable, the value supports variable substitution.
            var_set_parse(optarg, true);
            break;

        case 'X':
            // -X <key=value>: set a variable, the value doesn't support variable substitution.
            var_set_parse(optarg, false);
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

    if (option_select_mode)
    {
        if (displayTemplate == NULL)
        {
            fprintf(stderr, "Error: option_select mode enabled without a display_template specified.\n");
            print_usage();
            image_quit();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);

            sdl_do_quit();
            return EXIT_FAILURE;
        }

        if (ini_read(option_select_file, &option_parse, NULL))
        {
            print_usage();

            image_quit();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);

            sdl_do_quit();
            return EXIT_FAILURE;
        }

        if (root_option == NULL)
        {
            print_usage();

            image_quit();
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);

            sdl_do_quit();
            return EXIT_FAILURE;            
        }

        if (default_select != NULL)
        {
            Option_List *current_opt = root_option;
            Option_List *first_opt = root_option;

            do
            {
                if (strcasecmp(current_opt->id, default_select) == 0)
                    break;

                current_opt = current_opt->next;
            } while (current_opt != first_opt);

            root_option = current_opt;
            root_image = root_option->image_object;
        }
        else
        {
            // By default it will be on the last option, so go to the head (next).
            root_option = root_option->next;
            root_image = root_option->image_object;
        }

        fprintf(stderr, "= %s\n", root_option->id);
    }

    const char* db_file = NULL;
    if (db_file = SDL_getenv("SDL_GAMECONTROLLERCONFIG_FILE"))
    {
        SDL_GameControllerAddMappingsFromFile(db_file);
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
            fprintf(stderr, "loop\n");
            // Clear screen
            SDL_RenderClear(renderer);

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

    init_vars();
    return 0;
}

void sdl_do_quit()
{
    quit_vars();

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


void var_set_parse(const char *text, bool var_sub)
{
    if (text == NULL)
    {
        fprintf(stderr, "Error: Input text is NULL\n");
        return;
    }

    const char *delimiter = strchr(text, '=');
    if (delimiter == NULL)
    {
        fprintf(stderr, "Error: No '=' found in the input text \"%s\"\n", text);
        return;
    }

    // Calculate the lengths of the key and value
    size_t key_len = delimiter - text;
    size_t value_len = strlen(delimiter + 1);

    // Allocate memory for key and value
    char *key = (char *)ez_malloc(key_len + 1);
    char *value = (char *)ez_malloc(value_len + 1);

    // Copy the key and value into the allocated memory
    strncpy(key, text, key_len);
    key[key_len] = '\0';
    strcpy(value, delimiter + 1);

    if (var_sub)
    {
        char *new_value = sub_vars(value);
        set_var(key, new_value);
        free(new_value);
    }
    else
    {
        set_var(key, value);
    }

    // Free the allocated memory
    free(key);
    free(value);
}


void ini_parse(void *state, const char *key, const char *value)
{   // Callback for INI parsing, also used in getopt.
    if (strcasecmp(key, "image") == 0)
    {
        imageFallback = ! load_image(value);
    }
    else if (strcasecmp(key, "image_fallback") == 0)
    {
        if (imageFallback)
            imageFallback = ! load_image(value);
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
        fontFallback = ! load_font(value);
    }
    else if (strcasecmp(key, "font_fallback") == 0)
    {
        if (fontFallback)
            fontFallback = ! load_font(value);
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
    else if (strcasecmp(key, "set") == 0)
    {
        var_set_parse(value, true);
    }
    else if (strcasecmp(key, "set_strict") == 0)
    {
        var_set_parse(value, false);
    }
    else
    {
        fprintf(stderr, "Unknown INI: %s = %s\n", key, value);
    }
}


void option_parse(void *state, const char *key, const char *value)
{
    set_var("id", key);

    char *new_value = strdup(value);
    if (new_value == NULL)
        return;

    char *token = strtok(new_value, ";;");

    while (token != NULL)
    {
        fprintf(stderr, "- %s\n", token);
        var_set_parse(token, true);

        token = strtok(NULL, ";;");
    }

    free(new_value);

    Option_List *option_item = (Option_List*)ez_malloc(sizeof(Option_List));

    if (root_option == NULL)
    {
        // If the list is empty, initialize the root_option
        root_option = option_item;
        option_item->next = option_item;
        option_item->prev = option_item;
    }
    else
    {   
        // Insert the new item at the end of the doubly linked list
        Option_List *last = root_option->prev; // Get the last item in the list

        // Set the new item pointers
        option_item->next = root_option;      // New item points to the root
        option_item->prev = last;              // New item points to the last item

        // Update the last item and root item pointers
        last->next = option_item;              // Last item points to the new item
        root_option->prev = option_item;      // Root item points to the new item as the previous item
    }

    option_item->image_object = image_global_duplicate();
    option_item->id = strdup(key);

    root_image  = option_item->image_object;
    root_option = option_item;

    ini_read(displayTemplate, &ini_parse, NULL);
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
        lines[numLines++] = strdup(token);
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
    fprintf(stderr, "render_text_wrapped:\n");
    for (int i = 0; i < numLines; ++i)
    {
        fprintf(stderr, "- %s\n", lines[i]);
        SDL_Surface* tempSurface = TTF_RenderText_Blended_Wrapped(globalFont, lines[i], (SDL_Color){255, 255, 255, 255}, maxWidth);
        if (!tempSurface)
        {
            fprintf(stderr, "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
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


bool load_image(const char *imageFile)
{
    // Load image
    char *imageRef = sub_vars(imageFile);

    if (imageRef == NULL)
        return false;

    if (!file_exists(imageRef))
    {
        fprintf(stderr, "load_image: %s: file doesn't exist.\n", imageRef);
        return false;
    }

    SDL_Surface *imageSurface = IMG_Load(imageRef);
    free(imageRef);

    if (imageSurface == NULL)
    {
        fprintf(stderr, "IMG: Couldn't load %s: %s\n", imageRef, IMG_GetError());
        return false;
    }

    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    if (imageSurface == NULL)
    {
        return false;
    }

    Image_Object *image = image_create();

    image->imageTexture = imageTexture;

    calculate_texture_size(image->imageTexture, &image->imageRect, imageSize);
    calculate_texture_rect(image->imageTexture, &image->imageRect, imagePosition);

    return true;
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


bool load_font(const char *fontFile)
{
    int scaleSize = (int)(float)((screenHeight / 480.0f) * (float)fontSize);

    char *fontRef = sub_vars(fontFile);

    if (fontRef == NULL)
        return false;

    if (!file_exists(fontRef))
    {
        fprintf(stderr, "load_font: %s: file doesn't exist.\n", fontRef);
        return false;
    }

    TTF_Font *oldFont = globalFont;

    globalFont = TTF_OpenFont(fontRef, scaleSize);
    if (globalFont == NULL)
    {   // Restore old font.
        globalFont = oldFont;

        fprintf(stderr, "TTF: Couldn't load %s: %s\n", fontRef, TTF_GetError());
        return false;
    }

    if (oldFont != NULL)
    {
        free(globalFontName);
        TTF_CloseFont(oldFont);
    }

    globalFontName = fontRef;

    return true;
}


bool render_text(const char *text)
{
    char *textRef = sub_vars(text);
    if (textRef == NULL)
        return false;        

    SDL_Surface *imageSurface = render_text_wrapped(textRef);
    if (imageSurface == NULL)
    {
        fprintf(stderr, "TTF: Couldn't render \"%s\" -> \"%s\": %s\n", text, textRef, IMG_GetError());
        free(textRef);
        return false;
    }

    free(textRef);

    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    if (imageSurface == NULL)
    {
        return false;
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

    return true;
}


Image_Object *image_global_duplicate()
{
    Image_Object *current = global_image;
    Image_Object *object  = NULL;
    Image_Object *result  = NULL;
    Image_Object *last    = NULL;

    while (current != NULL)
    {
        Image_Object *object = (Image_Object *)ez_malloc(sizeof(Image_Object));

        memcpy(object, current, sizeof(Image_Object));

        object->next = NULL;
        object->duplicate = true;

        if (last != NULL)
            last->next = object;

        if (result == NULL)
            result = object;

        last = current;
        current = current->next;
    }

    return result;
}


Image_Object *image_create()
{
    Image_Object *image = (Image_Object *)ez_malloc(sizeof(Image_Object));

    image->drawColor.r = 255;
    image->drawColor.g = 255;
    image->drawColor.b = 255;
    image->drawColor.a = 255;

    image->duplicate = false;

    if (global_image == NULL && root_option == NULL)
    {
        global_image = image;
    }

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
    Image_Object *current_img = global_image;
    Image_Object *next_img = NULL;

    while (current_img != NULL)
    {
        next_img = current_img->next;

        if (!current_img->duplicate && current_img->imageTexture != NULL)
            SDL_DestroyTexture(current_img->imageTexture);

        free(current_img);

        current_img = next_img;
    }

    if (root_option != NULL)
    {
        Option_List *current_opt = root_option;
        Option_List *next_opt;

        // Break the double linked list.
        current_opt->prev->next = NULL;

        while (current_opt)
        {
            next_opt = current_opt->next;

            current_img = current_opt->image_object;

            while (current_img != NULL)
            {
                next_img = current_img->next;

                if (!current_img->duplicate && current_img->imageTexture != NULL)
                    SDL_DestroyTexture(current_img->imageTexture);

                free(current_img);

                current_img = next_img;
            }

            free(current_opt->id);
            free(current_opt);

            current_opt = next_opt;
        }
    }
}
