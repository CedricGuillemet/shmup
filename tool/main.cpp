#include "imgui.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include "imgui_internal.h"
#include "moviePlayback.h"
#include <string>
#include "Movie.h"

/*
 - asset issue: face colors second part of tunnel_light
 - LOOP count or CLEARED
 - FORE ON/OFF
 
 - transition/warp token
 - per frame debug display (frame count, vt count, colors, frame size)
 */
Movie movie;

struct Triangle
{
    int16_t ax,ay,bx,by,cx,cy;
    uint32_t color;
};

uint32_t BGRA(uint32_t v)
{
    return (v & 0xFF000000) + ((v & 0xFF) << 16) + (v & 0xFF00) + ((v & 0xFF0000)>>16);
}

std::vector<Triangle> triangles;
std::vector<Triangle> backgroundTriangles;
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

void DrawTriangleMovie(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy, uint8_t color)
{
    if (renderingBackground)
    {
        backgroundTriangles.push_back({ax,ay,bx,by,cx,cy, BGRA(palette[color])});
    }
    else
    {
        triangles.push_back({ax,ay,bx,by,cx,cy, BGRA(palette[color])});
    }
}

bool drawBackground(true), drawForeground(true);

void Draw(int width, int height, const char* buttonName)
{
    int factor = 2;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available

    draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + 319 * factor, canvas_pos.y + 199 * factor));
    draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + 319 * factor, canvas_pos.y + 199 * factor), 0xFFFF00FF);
    
    std::vector<Triangle>* batches[] = {&backgroundTriangles, &triangles};
    for(int i = (backgroundVisible?0:1); i < 2; i++)
    {
        if ((i == 0 && !drawBackground) || (i && !drawForeground))
        {
            continue;
        }
        const auto& batch = batches[i];
        int x = (i == 0) ? scrollx : 0;
        int y = (i == 0) ? scrolly : 0;
        for(auto triangle : *batch)
        {
            draw_list->AddTriangleFilled(ImVec2(canvas_pos.x + (triangle.cx - x) * factor,
                                                canvas_pos.y + (triangle.cy - y) * factor),
                                         ImVec2(canvas_pos.x + (triangle.bx - x) * factor,
                                                canvas_pos.y + (triangle.by - y) * factor),
                                         ImVec2(canvas_pos.x + (triangle.ax - x) * factor,
                                                canvas_pos.y + (triangle.ay - y) * factor),
                                         triangle.color);
        }
    }
    draw_list->PopClipRect();
    ImGui::InvisibleButton(buttonName, ImVec2(width * factor, height * factor));
}

void BeginBackground(unsigned short width, unsigned short height)
{
    backgroundTriangles.clear();
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

void frame()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(800, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("Movie");
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
        triangles.clear();
        RenderMovieFrame(frameIndex);
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
                triangles.clear();
                RenderMovieFrame();
                frameIndex ++;
            }
            else
            {
                playing = false;
            }
        }
    }
    Draw(320, 200, "InvButton");
    ImGui::End();
}

int main(int, char **) {
    CompileMovie();
    imgui_app(frame, "shmup Tool", 1024, 768);
    return 0;
}
