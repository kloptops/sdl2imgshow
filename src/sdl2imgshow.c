// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>


void print_usage()
{
    printf("Usage: sdl2imgshow -i <image_file> -f <font_file> -t <text> [-c <R,G,B>] [-s <font_size>]\n");
}


int main(int argc, char *argv[])
{
    char *imageFile = NULL;
    char *fontFile = NULL;
    char *text = NULL;
    SDL_Color textColor = {255, 255, 255, 255};
    int fontSize = 24;

    int opt;
    while ((opt = getopt(argc, argv, "i:f:t:c:s:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            imageFile = optarg;
            break;
        case 'f':
            fontFile = optarg;
            break;
        case 't':
            text = optarg;
            break;
        case 'c':
            sscanf(optarg, "%d,%d,%d", &textColor.r, &textColor.g, &textColor.b);
            break;
        case 's':
            fontSize = atoi(optarg);
            break;
        default: /* '?' */
            print_usage();
            exit(EXIT_FAILURE);
        }
    }

    // Check if required arguments are provided
    if (imageFile == NULL || fontFile == NULL || text == NULL)
    {
        print_usage();
        exit(EXIT_FAILURE);
    }

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *imageTexture;
    SDL_Surface *imageSurface;
    SDL_Rect imageRect;
    SDL_Surface *textSurface;
    SDL_Texture *textTexture;
    SDL_Rect textRect;
    SDL_Event event;
    int quit = 0;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_image
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        printf("IMG_Init Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialize SDL_ttf
    if (TTF_Init() != 0)
    {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

   // Get screen resolution
    SDL_DisplayMode dm;
    if (SDL_GetCurrentDisplayMode(0, &dm) != 0)
    {
        printf("SDL_GetCurrentDisplayMode Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    int screenWidth = dm.w;
    int screenHeight = dm.h;

    // Create window
    window = SDL_CreateWindow("SDL2 Image Show",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screenWidth, screenHeight, SDL_WINDOW_FULLSCREEN);

    if (window == NULL)
    {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL)
    {
        SDL_DestroyWindow(window);
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Load image
    imageSurface = IMG_Load(imageFile);
    if (imageSurface == NULL)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        printf("IMG_Load Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    // Create texture from image
    imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);
    if (imageTexture == NULL)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Get image dimensions
    SDL_QueryTexture(imageTexture, NULL, NULL, &imageRect.w, &imageRect.h);
    imageRect.x = (screenWidth - imageRect.w) / 2;
    imageRect.y = (screenHeight - imageRect.h) / 2;

    // Load font
    TTF_Font* font = TTF_OpenFont(fontFile, fontSize);
    if (font == NULL)
    {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyTexture(imageTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Render text
    textSurface = TTF_RenderText_Solid(font, text, textColor);
    if (textSurface == NULL)
    {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        SDL_DestroyTexture(imageTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    if (textTexture == NULL)
    {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_DestroyTexture(imageTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.x = (screenWidth - textRect.w) / 2;
    textRect.y = (screenHeight - textRect.h) / 2;

    // Wait for quit event
    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
        }

        // Clear screen
        SDL_RenderClear(renderer);

        // Render image
        SDL_RenderCopy(renderer, imageTexture, NULL, &imageRect);

        // Render text
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // Update screen
        SDL_RenderPresent(renderer);

        // 10 frames a second is fine.
        SDL_Delay(16);
    }

    // Clean up
    SDL_DestroyTexture(imageTexture);
    SDL_DestroyTexture(textTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
