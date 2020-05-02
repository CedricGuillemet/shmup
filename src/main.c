#include <SDL.h>
#include <stdio.h>
#include <memory.h>


#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_FACTOR 4

typedef unsigned char bool;
#define true 1
#define false 0

#include "tga.h"
#include "types.h"
#include "tables.h"
#include "display.h"
#include "hud.h"
#include "effects.h"
#include "bullets.h"
#include "enemies.h"
#include "ship.h"
#include "orchestra.h"
#include "geometry.h"

#include "triggers.h"
#include "level.h"

SDL_Window* window = NULL;
SDL_Surface* screenSurface = NULL;

static bool init() 
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
        return false;
    }
    window = SDL_CreateWindow(
        "shmup",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH * SCREEN_FACTOR, SCREEN_HEIGHT * SCREEN_FACTOR,
        SDL_WINDOW_SHOWN
    );
    if (window == NULL)
    {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return false;
    }
    screenSurface = SDL_GetWindowSurface(window);
    if (screenSurface == NULL)
    {
        fprintf(stderr, "could not get window: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

static void close() {
    SDL_FreeSurface(screenSurface); screenSurface = NULL;
    SDL_DestroyWindow(window); window = NULL;
    SDL_Quit();
}





void BlitSurface(SDL_Surface* surface)
{
    Uint32* target_pixel = (Uint32*)surface->pixels;
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            for (int sy = 0; sy < SCREEN_FACTOR; sy++)
            {
                for (int sx = 0; sx < SCREEN_FACTOR; sx++)
                {
                    target_pixel[(y * SCREEN_FACTOR + sy) * SCREEN_WIDTH * SCREEN_FACTOR + x * SCREEN_FACTOR + sx] = palette[buffer[y * SCREEN_WIDTH + x]];
                }
            }
            
        }
    }
}



void EndianSwap(uint32_t *buffer, int lengthInBytes)
{
    for (int i = 0; i < lengthInBytes/4;i++)
    {
        uint32_t v = buffer[i];
        buffer[i] = ((v&0xFF)<<16) + (v & 0xFF00) + ((v & 0xFF0000) >> 16);
    }
}

struct Input_t
{
    bool left;
    bool right;
    bool up;
    bool down;
    bool fire;
    bool switchColor;
    bool discharge;
};

struct Input_t Input;

int main(int argc, char** argv)
{
    if (!init()) return 1;
    
    struct gl_texture_t* paletteFile = ReadTGAFile("palette.tga");
    memcpy(palette, paletteFile->texels, 256 * 4);
    EndianSwap(palette, 256*4);
    memset(buffer, 17, sizeof(buffer));
    memset(&Input, 0, sizeof(Input));
    angle = FromInt(0);

    SpawnShip();
    /*
    FILE *fp = fopen("tan.txt", "wt");
    for (int i = 0;i<1024;i++)
    {
        float angle = (i / 1024.f) * 3.141592f;
        fprintf(fp, "0x%x,\n", (int)(tanf(angle) * 65536.f));
    }
    fclose(fp);
    */

    SDL_Surface* img = SDL_CreateRGBSurface(0, SCREEN_WIDTH * SCREEN_FACTOR, SCREEN_HEIGHT * SCREEN_FACTOR, 32, 0, 0, 0, 0);
    if (img == NULL) return 1;

    unsigned int lastTime = 0, currentTime;
    bool done = false;
    while(!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_LEFT:
                    Input.left = true;
                    break;
                case SDLK_RIGHT:
                    Input.right = true;
                    break;
                case SDLK_UP:
                    Input.up = true;
                    break;
                case SDLK_DOWN:
                    Input.down = true;
                    break;
                case SDLK_z:
                    Input.fire = true;
                    break;
                case SDLK_x:
                    Input.switchColor = true;
                    break;
                case SDLK_c:
                    Input.discharge = true;
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) 
                {
                case SDLK_LEFT:
                    Input.left = false;
                    break;
                case SDLK_RIGHT:
                    Input.right = false;
                    break;
                case SDLK_UP:
                    Input.up = false;
                    break;
                case SDLK_DOWN:
                    Input.down = false;
                    break;
                case SDLK_z:
                    Input.fire = false;
                    break;
                case SDLK_x:
                    Input.switchColor = false;
                    break;
                case SDLK_c:
                    Input.discharge = false;
                    break;
                }
                break;
            case SDL_QUIT:
                done = true;
                break;
            }
        }

        

        
        TickHUD(Ship.jauge, GlobalFrame);

        // tick game
        if (freezeFrame == 0)
        {
            TickShip(Input.left, Input.right, Input.up, Input.down, Input.fire, Input.switchColor, Input.discharge);
            TickOrchestra();
            TickBullets();
            TickEnemies();
            TickEffects();
        

            TickBulletsDamagingEnemies();
            TickBulletsDamagingShip();

            DrawLevel();
            DrawBullets(0);
            DrawShip();
            DrawEnemies();
            DrawBullets(1);
            DrawEffects();
            DrawHUD(Ship.jauge);
        

            //set_pixel(160,100, 16);
            BlitSurface(img);
            SDL_BlitSurface(img, NULL, screenSurface, NULL);
            SDL_UpdateWindowSurface(window);
        }

        currentTime = SDL_GetTicks();
        while (SDL_GetTicks() - lastTime < 16) 
        {
        };

        currentTime = SDL_GetTicks();
        lastTime = currentTime;
        
    };
    
    SDL_FreeSurface(img); img = NULL;
    close();

    return 0;
}