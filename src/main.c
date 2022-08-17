#include <SDL.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_FACTOR 4

#include "tga.h"
#include "types.h"
#include "tables.h"
#include "constants.h"
#include "meshes.h"
#include "palette.h"
#include "geometry.h"
#include "score.h"
#include "display.h"
#include "hud.h"
#include "effects.h"
#include "paths.h"
#include "bullets.h"
#include "ship.h"
#include "enemies.h"

#include "orchestra.h"

#include "triggers.h"
#include "level.h"
#include "moviePlayback.h"
#include "states.h"
#include "record.h"

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

int main(int argc, char** argv)
{
    if (!init()) return 1;
    

    SpritesInit();

    memset(buffer, 17, sizeof(buffer));

    struct Input_t Input;
    memset(&Input, 0, sizeof(Input));
    angle = FromInt(0);

    PrecomputePaths();
    PrecomputeSpawns();

    SpawnShip();

    bool playingback = false;//LoadRecord("Record.bin");

    SDL_Surface* img = SDL_CreateRGBSurface(0, SCREEN_WIDTH * SCREEN_FACTOR, SCREEN_HEIGHT * SCREEN_FACTOR, 32, 0, 0, 0, 0);
    if (img == NULL) return 1;

    unsigned int lastTime = 0, currentTime;
    bool done = false;
    while(!done)
    {
        struct InputRecord* currentRecord = &Record[RecordEntryCount];
        memset(currentRecord, 0, sizeof(struct InputRecord));
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                case SDLK_LEFT:
                    if (!playingback && !Input.left)
                    {
                        Input.left = true;
                        currentRecord->leftChanged = true;
                    }
                    break;
                case SDLK_RIGHT:
                    if (!playingback && !Input.right)
                    {
                        Input.right = true;
                        currentRecord->rightChanged = true;
                    }
                    break;
                case SDLK_UP:
                    if (!playingback && !Input.up)
                    {
                        Input.up = true;
                        currentRecord->upChanged = true;
                    }
                    break;
                case SDLK_DOWN:
                    if (!playingback && !Input.down)
                    {
                        Input.down = true;
                        currentRecord->downChanged = true;
                    }
                    break;
                case SDLK_z:
                    if (!playingback && !Input.fire)
                    {
                        Input.fire = true;
                        currentRecord->fireChanged = true;
                    }
                    break;
                case SDLK_x:
                    if (!playingback && !Input.switchColor)
                    {
                        Input.switchColor = true;
                        currentRecord->switchColorChanged = true;
                    }
                    break;
                case SDLK_c:
                    if (!playingback && !Input.discharge)
                    {
                        Input.discharge = true;
                        currentRecord->dischargeChanged = true;
                    }
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) 
                {
                case SDLK_LEFT:
                    if (!playingback && Input.left)
                    {
                        Input.left = false;
                        currentRecord->leftChanged = true;
                    }
                    break;
                case SDLK_RIGHT:
                    if (!playingback && Input.right)
                    {
                        Input.right = false;
                        currentRecord->rightChanged = true;
                    }
                    break;
                case SDLK_UP:
                    if (!playingback && Input.up)
                    {
                        Input.up = false;
                        currentRecord->upChanged = true;
                    }
                    break;
                case SDLK_DOWN:
                    if (!playingback && Input.down)
                    {
                        Input.down = false;
                        currentRecord->downChanged = true;
                    }
                    break;
                case SDLK_z:
                    if (!playingback && Input.fire)
                    {
                        Input.fire = false;
                        currentRecord->fireChanged = true;
                    }
                    break;
                case SDLK_x:
                    if (!playingback && Input.switchColor)
                    {
                        Input.switchColor = false;
                        currentRecord->switchColorChanged = true;
                    }
                    break;
                case SDLK_c:
                    if (!playingback && Input.discharge)
                    {
                        Input.discharge = false;
                        currentRecord->dischargeChanged = true;
                    }
                    break;
                }
                break;
            case SDL_QUIT:
                done = true;
                break;
            }
        }
        if (playingback)
        {
            HandlePlayback(&Input);
        }
        else
        {
            if ( *(unsigned int*)currentRecord )
            {
                currentRecord->frame = GlobalFrame;
                RecordEntryCount++;
            }
        }

        
        GameLoop(Input);
        /*
        static int index = 0;
        static int flick = 0;
        flick ++;
        if ((flick &3) == 0)
        {
        index ++;
        index %= 3;
        }
        DrawSprite(V2FromInt(160,100), &tearShotWhite[index * 16 * 16 * 16], 16, 16, false);
        */

        BlitSurface(img);
        SDL_BlitSurface(img, NULL, screenSurface, NULL);
        SDL_UpdateWindowSurface(window);
        
        
        currentTime = SDL_GetTicks();
        while (SDL_GetTicks() - lastTime < 16) 
        {
        };
        GlobalFrame++;
        
        currentTime = SDL_GetTicks();
        lastTime = currentTime;
        
    };
    
    SDL_FreeSurface(img); img = NULL;
    close();

    if (!playingback)
    {
        SaveRecord("Record.bin");
    }
    return 0;
}
