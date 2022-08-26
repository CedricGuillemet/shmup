#include "imgui.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include "imgui_internal.h"
#include <string>
#include "sokol_gfx.h"


extern "C" {
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_FACTOR 4

#include "moviePlayback.h"
#include "tga.h"
#include "types.h"
#include "tables.h"
#include "glyph.h"
#include "trigo.h"
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
#include "sprites.h"
}
#include "Movie.h"
extern "C" {
/*
 - enemy content
 - shoot emitting
 - path evaluation
 - sub functions (?)
 
 - per frame debug display (frame count, vt count, colors, frame size)
 */
Movie movie;

bool renderingBackground(false);
bool backgroundVisible(false);
uint16_t backgroundWidth, backgroundHeight;
void BackgroundVisible(bool visible)
{
    backgroundVisible = visible;
}

uint32_t temp_bitmap[SCREEN_WIDTH * SCREEN_HEIGHT];
uint8_t backgroundBitmap[SCREEN_WIDTH * SCREEN_HEIGHT];
uint8_t cachedScreen[SCREEN_WIDTH * SCREEN_HEIGHT];

int frameIndex(-1);
int frameIndexStartWarp;
bool drawBackground(true), drawForeground(true);
bool warpOn(false), warpStripesOn(false), warpBackgroundOn(false);

void SetWarp(bool enabled, int warpFrameIndex)
{
    frameIndexStartWarp = warpFrameIndex;
    warpOn = enabled;
}
void SetWarpStripes(bool enabled) { warpStripesOn = enabled; }
void SetWarpBackground(bool enabled) { warpBackgroundOn = enabled; }
void BeginBackground(unsigned short width, unsigned short height)
{
    renderingBackground = true;
    backgroundWidth = width;
    backgroundHeight = height;
}

void StopBackground()
{
    renderingBackground = false;
    memcpy(backgroundBitmap, buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
}

void RenderBackground(int16_t x, int16_t y)
{
    int rw = backgroundWidth - x;
    if (rw > 320) rw = 320;
    if (rw <= 0) return;
    int rh = backgroundHeight - y;
    if (rh > 200) rh = 200;
    if (rh <= 0) return;
    
    for (int h = 0; h < rh; h++)
    {
        for (int w = 0; w < rw; w++)
        {
            int dst = h * 320 + w;
            int src = (h + y) * backgroundWidth + (w + x);
            buffer[dst] = backgroundBitmap[src];
        }
    }
}

int EnemiesCleared()
{
    return 1;
}

void DoSpawn(MovieSpawn* spawn)
{
    SpawnEnemy(EnemyTypeWhite/*spawn->type*/, V2FromInt(spawn->x, spawn->y), spawn->pathIndex, 0, 0, 0);
}

void TickFrame()
{
    TickEnemies();
}

bool playing(false), nextFrame(true);

int totalFrameCount(0);
std::string errorMessage;

void CompileMovie()
{
    if (movie.ParseScript("movie.txt"))
    {
        movie.WriteMovie("movie.bin");
        ReadMovie("movie.bin");
        
        totalFrameCount = GetMovieFrameCount();
        frameIndex = (frameIndex >= totalFrameCount -1) ? totalFrameCount-1 : frameIndex;
        playing = false;
        
        if (frameIndex >= 0)
        {
            ClearEnemies();
            RenderMovieSingleFrame(frameIndex);
        }
        else
        {
            nextFrame = true;
        }
    }
    errorMessage = movie.GetParsingError();
}

ImTextureID ftex = 0;
sg_image skImage;

void updateTex()
{
    if (!ftex)
    {
        for (int i = 0; i < 320 * 200; i++)
        {
            temp_bitmap[i] = i * i;
        }
        
        sg_image_desc img_desc;
        //_simgui_clear(&img_desc, sizeof(img_desc));
        memset(&img_desc, 0, sizeof(img_desc));
        img_desc.usage = SG_USAGE_STREAM;
        img_desc.width = 320;
        img_desc.height = 200;
        img_desc.pixel_format = SG_PIXELFORMAT_BGRA8;
        img_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        img_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        img_desc.min_filter = SG_FILTER_LINEAR;
        img_desc.mag_filter = SG_FILTER_LINEAR;
        img_desc.label = "game";
        skImage = sg_make_image(&img_desc);
        ftex = (ImTextureID)(uintptr_t) skImage.id;
    }
    else
    {
        for (int i = 0;i<320*200;i++)
        {
            temp_bitmap[i] = palette[buffer[i]];
        }
        
        sg_image_data data;
        memset(&data, 0, sizeof(data));
        data.subimage[0][0].ptr = temp_bitmap;
        data.subimage[0][0].size = (size_t)(320 * 200) * sizeof(uint32_t);
        sg_update_image(skImage, data);
    }
}

} // C
void frame()
{
    updateTex();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(800, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("Movie");
    ImGui::Image(ftex, ImVec2(SCREEN_WIDTH * 2, SCREEN_HEIGHT * 2));
    if (ImGui::Button("Reload"))
    {
        CompileMovie();
    }
    if (errorMessage.size())
    {
        ImGui::LabelText("Error parsing movie text !", "%s", errorMessage.c_str());
        ImGui::End();
        return;
    }
    if (!totalFrameCount)
    {
        ImGui::End();
        return;
    }
    if (ImGui::SliderInt("Frame", &frameIndex, 0, totalFrameCount-1))
    {
        ClearEnemies();
        RenderMovieSingleFrame(frameIndex);
    }
    ImGui::SameLine();
    ImGui::LabelText("End", "%d", totalFrameCount);
    if (ImGui::Button("Next Frame"))
    {
        nextFrame = true;
    }
    if (ImGui::Button(playing? "Stop" : "Play"))
    {
        playing = !playing;
    }
    
    ImGui::Checkbox("Draw Background", &drawBackground);
    ImGui::Checkbox("Draw Foreground", &drawForeground);
    
    if (nextFrame || playing)
    {
        TickEnemies();
        frameIndex ++;
        nextFrame = false;
        
        static int other = 1;
        other ++;
        if (other % MOVIE_TO_GAMEPLAY_RATIO == 0)
        {
            if (frameIndex < totalFrameCount-1)
            {
                RenderMovieFrame();
                memcpy(cachedScreen, buffer, SCREEN_WIDTH * SCREEN_HEIGHT);
            }
            else
            {
                playing = false;
            }
        } else {
            memcpy(buffer, cachedScreen, SCREEN_WIDTH * SCREEN_HEIGHT);
        }
    }
    
    ComputeMatrices();
    DrawEnemies();
    if (warpOn)
    {
        int warpFrame = frameIndex - frameIndexStartWarp;
        if (warpFrame >= 400)
        {
            warpFrame = 399;
        }
        GameWarping(warpFrame, warpStripesOn, warpBackgroundOn);
    }
    ImGui::End();
}

int main(int, char **) {
    CompileMovie();
    SpritesInit();
    PrecomputePaths();
    imgui_app(frame, "shmup Tool", 1024, 768);
    return 0;
}
