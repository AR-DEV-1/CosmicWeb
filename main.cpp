// Cosmic Web

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cmath>
#include
#define STB_TRUETYPE_IMPLEMENTATION
#include "include/stb_truetype.h"

// Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

int main(int argc, char* args[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("SDL + stb_truetype", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Get window surface
    SDL_Surface* screenSurface = SDL_GetWindowSurface(window);

    // Load font file
    FILE* fontFile = fopen("path/to/font.ttf", "rb");
    if (!fontFile) {
        printf("Failed to load font file\n");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Load font data
    fseek(fontFile, 0, SEEK_END);
    long size = ftell(fontFile);
    fseek(fontFile, 0, SEEK_SET);

    unsigned char* fontBuffer = (unsigned char*)malloc(size);
    fread(fontBuffer, size, 1, fontFile);
    fclose(fontFile);

    // Initialize font
    stbtt_fontinfo font;
    stbtt_InitFont(&font, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0));

    // Generate text bitmap
    int w, h, x, y;
    float scale = stbtt_ScaleForPixelHeight(&font, 24); // 24px height

    unsigned char* bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, 'A', &w, &h, &x, &y);

    // Copy bitmap to SDL surface
    SDL_Surface* textSurface = SDL_CreateRGBSurfaceFrom(bitmap, w, h, 8, w, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_BlitSurface(textSurface, NULL, screenSurface, NULL);

    // Update window
    SDL_UpdateWindowSurface(window);

    // Wait for 2 seconds
    SDL_Delay(2000);

    // Free resources and close SDL
    SDL_FreeSurface(textSurface);
    free(bitmap);
    free(fontBuffer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
