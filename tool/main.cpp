#include "imgui.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include "imgui_internal.h"
#include <string>
#include "Movie.h"
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

/*
 - LOOP count or CLEARED
 - FORE ON/OFF
 - transition/warp token
 - cache rendered background
 - per frame debug display (frame count, vt count, colors, frame size)
 */
Movie movie;

bool renderingBackground(false);
bool backgroundVisible(false);
int16_t scrollx, scrolly;
void BackgroundVisible(bool visible)
{
    backgroundVisible = visible;
}

void SetScroll(int16_t x, int16_t y)
{
    scrollx = x;
    scrolly = y;
}

uint32_t temp_bitmap[SCREEN_WIDTH * SCREEN_HEIGHT];
bool drawBackground(true), drawForeground(true);

void BeginBackground(unsigned short width, unsigned short height)
{
    renderingBackground = true;
}

void StopBackground()
{
    renderingBackground = false;
}

bool playing(false), nextFrame(true);
int frameIndex(-1);
int totalFrameCount(0);
std::string errorMessage;

void CompileMovie()
{
    if (movie.ParseScript("movie.txt"))
    {
        movie.WriteMovie("movie.bin");
        ReadMovie("movie.bin");
        
        totalFrameCount = GetMovieFrameCount();
        frameIndex = -1;
        playing = false;
        nextFrame = true;
    }
    errorMessage = movie.GetParsingError();
}

ImTextureID ftex = 0;
sg_image skImage;

void updateTex()
{
    if (!ftex)
    {
        for (int i = 0;i<320*200;i++)
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
        return;
    }
    if (ImGui::SliderInt("Frame", &frameIndex, 0, totalFrameCount-1))
    {
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
        static int other = 0;
        other ++;
        if (other %3 == 0)
        {
            nextFrame = false;
            
            if (frameIndex < totalFrameCount-1)
            {
                RenderMovieFrame();
                frameIndex ++;
            }
            else
            {
                playing = false;
            }
        }
    }

    ImGui::End();
}

int main(int, char **) {
    CompileMovie();
    imgui_app(frame, "shmup Tool", 1024, 768);
    return 0;
}
