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
#include "ship.h"
#include "geometry.h"

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

void set_pixel(int x, int y, uint8_t pixel)
{
    if (y < 0 || y >= SCREEN_HEIGHT || x < 0 || x >= SCREEN_WIDTH)
    {
        return;
    }
    buffer[y * SCREEN_WIDTH + x] = pixel;
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
};

struct Input_t Input;
struct Fixed angle;
int main(int argc, char** argv)
{
    if (!init()) return 1;
    
    struct gl_texture_t* paletteFile = ReadTGAFile("palette.tga");
    memcpy(palette, paletteFile->texels, 256 * 4);
    EndianSwap(palette, 256*4);
    memset(buffer, 17, sizeof(buffer));
    memset(&Input, 0, sizeof(Input));
    angle = FromInt(0);
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
                }
                break;
            case SDL_QUIT:
                done = true;
                break;
            }
        }

        // tick game
        TickShip(Input.left, Input.right, Input.up, Input.down);

        // clear
        memset(buffer, 17, sizeof(buffer));


        // test geom3d
        struct Fixed fovy, aspect, zn, zf;
        SetFixed(&fovy, 60622);
        SetFixed(&aspect, (65536 * 4) / 3);
        SetFixed(&zn, 32768);
        SetFixed(&zf, 65536*1000);
        struct Matrix_t perspective = PerspectiveFov(fovy, aspect, zn, zf);

        
        angle = Add(angle, FromFixed(600));
        struct Fixed circular = RadianToCircular(angle);
        struct Vector3 eye = V3Mul(V3FromFixed(Cosine(circular), FromInt(0), Sine(circular)), FromInt(10));


        struct Matrix_t view = LookAt(eye, V3FromInt(0, 0, 0), V3FromInt(0, 1, 0));
        struct Matrix_t clipSpaceTo2D = IdentityMatrix();
        clipSpaceTo2D.v[0] = FromInt(160);
        clipSpaceTo2D.v[5] = FromInt(100);
        clipSpaceTo2D.v[12] = FromInt(160);
        clipSpaceTo2D.v[13] = FromInt(100);
        struct Matrix_t perspectiveScreen = MulMatrix(perspective, clipSpaceTo2D);
        struct Matrix_t vp = MulMatrix(view, perspectiveScreen);
        

        char v[8 * 3] = {
            -1,-1,-1,
            -1,-1, 1,
             1,-1, 1,
             1,-1,-1,

            -1, 1,-1,
            -1, 1, 1,
             1, 1, 1,
             1, 1,-1,
        };

        struct Vector2 screenpos[8];
        for (int i = 0;i<8;i++)
        {
            screenpos[i] = TransformV3I8(&vp, &v[i * 3]);
            set_pixel(screenpos[i].x.integer, screenpos[i].y.integer, 15);
        }




        struct Vector2 halfExtend;
        V2SetInt(&halfExtend, 16, 16);
        Rectangle(Ship.position, halfExtend, 16);

        //set_pixel(160,100, 16);
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

    return 0;
}