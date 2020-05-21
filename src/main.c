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
#include "constants.h"
#include "meshes.h"
#include "palette.h"
#include "geometry.h"
#include "display.h"
#include "hud.h"
#include "effects.h"
#include "paths.h"
#include "bullets.h"
#include "enemies.h"
#include "ship.h"
#include "orchestra.h"


#include "triggers.h"
#include "level.h"
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

void EndianSwap(uint32_t *buffer, int lengthInBytes)
{
    for (int i = 0; i < lengthInBytes/4;i++)
    {
        uint32_t v = buffer[i];
        buffer[i] = ((v&0xFF)<<16) + (v & 0xFF00) + ((v & 0xFF0000) >> 16);
    }
}

//struct Input_t Input;
uint8_t* remappedShootWhite;
uint8_t* remappedShootBlack;

uint8_t* RemapBitmap(struct gl_texture_t* texture)
{
    int texSize = texture->width * texture->height;
    EndianSwap((uint32_t*)texture->texels, texSize * 4);

    unsigned char *bitmap = (unsigned char*)malloc(texSize);
    memset(bitmap, 0, texSize);

    int index = 0;
    for (int y = texture->height - 1; y >= 0; y--)
    //for (int y = 0; y < texture->height; y++)
    {
        for (int x = 0; x < texture->width; x++)
        {
            uint32_t color = ((uint32_t*)texture->texels)[y * texture->width + x];
            for (int j = 1; j < 256; j++)
            {
                if ( palette[j] == color)
                {
                    bitmap[index] = j;
                    break;
                }
            }
            index++;
        }
    }

    return bitmap;
}

void ConvertFontTextureToFontGlyphs(uint32_t* texture)
{
    memset(glyph, 0, sizeof(glyph));
    int out = 0;
    for (int h = 5; h >= 0; h--)
    {
        int offsetH = (h * 8 -1) * 21 * 8;
        for (int g = 0; g < 21; g++)
        {
            int offset = offsetH + g * 8;
            for (int y = 0;y<8;y++)
            {
                for (int x = 0;x<8;x++)
                {
                    int index = (offset - y * 21 * 8 + x);
                    if (texture[index]&0xFF)
                    {
                        glyph[out] |= 1 << x;
                    }
                }
                out++;
            }
        }
    }
}

void RotateSprite(uint8_t *source, int width, int height, uint8_t* destination, struct Fixed angle)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0;x < width; x++)
        {
            struct Vector2 coord = V2FromInt(x - width / 2, y - height / 2);
            struct Vector2 rot = V2Rotate(coord, angle);
            int py = rot.y.integer + height / 2;
            int px = rot.x.integer + width / 2;
            if (py < 0 || py >= height || px < 0 || px >= width)
            {
                *destination++ = 0;
            }
            else
            {
                *destination++ = source[py * width + px];
            }
        }
    }
}

uint8_t tearShotWhite[3 * 16 * 16 * 16];
uint8_t tearShotBlack[3 * 16 * 16 * 16];

int main(int argc, char** argv)
{
    if (!init()) return 1;
    
    struct gl_texture_t* paletteFile = ReadTGAFile("palette.tga");
    memcpy(palette, paletteFile->texels, 256 * 4);
    EndianSwap(palette, 256*4);
    memcpy(paletteSource, palette, 256*4);

    struct gl_texture_t* fontFile = ReadTGAFile("font.tga");
    ConvertFontTextureToFontGlyphs((uint32_t*)fontFile->texels);

    struct gl_texture_t* shootFileWhite = ReadTGAFile("shootWhite.tga");
    struct gl_texture_t* shootFileBlack = ReadTGAFile("shootBlack.tga");
    
    
    remappedShootWhite = RemapBitmap(shootFileWhite);
    remappedShootBlack = RemapBitmap(shootFileBlack);

    for (int j = 0; j < 6; j++)
    {
        static const char *tearShootFileNames[6] = {"bulletTear0Black.tga","bulletTear1Black.tga", "bulletTear2Black.tga",
            "bulletTear0White.tga","bulletTear1White.tga", "bulletTear2White.tga"};
        struct gl_texture_t* shootTear = ReadTGAFile(tearShootFileNames[j]);
        uint8_t* remappedShootTear = RemapBitmap(shootTear);

        for (int i = 0; i < 16; i++)
        {
            struct Fixed angleSpr = RadianToCircular(Mul(FromInt(i), FromFixed(80)));
            int index = j * 16 * 16 * 16 + i * 16 * 16;
            RotateSprite(remappedShootTear, 16, 16, &((j < 3) ? tearShotWhite : tearShotBlack)[index], angleSpr);
        }
    }



    memset(buffer, 17, sizeof(buffer));

    struct Input_t Input;
    memset(&Input, 0, sizeof(Input));
    angle = FromInt(0);

    PrecomputePaths();
    PrecomputeSpawns();

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

        static int index = 0;
        static int flick = 0;
        flick ++;
        if ((flick &3) == 0)
        {
        index ++;
        index %= 3;
        }
        DrawSprite(V2FromInt(160,100), &tearShotWhite[index * 16 * 16 * 16], 16, 16, false);


        BlitSurface(img);
        SDL_BlitSurface(img, NULL, screenSurface, NULL);
        SDL_UpdateWindowSurface(window);
        
        
        currentTime = SDL_GetTicks();
        while (SDL_GetTicks() - lastTime < 16) 
        {
        };
        
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