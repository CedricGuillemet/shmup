#include "imgui.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include "imgui_internal.h"
//#include "mesh.h"
#include "moviePlayback.h"
//#include "gltfImport.h"
#include <string>
#include "Movie.h"

Movie movie;

struct Triangle
{
    int16_t ax,ay,bx,by,cx,cy;
    uint32_t color;
};

std::vector<Triangle> triangles;

void DrawTriangleMovie(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy, uint8_t color)
{
    triangles.push_back({ax,ay,bx,by,cx,cy, palette[color]});
}

void Draw(int width, int height, const char* buttonName)
{
    int factor = 2;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available

    draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + 320, canvas_pos.y + 200));
    for(auto triangle : triangles)
    {
        draw_list->AddTriangleFilled(ImVec2(triangle.cx + canvas_pos.x, canvas_pos.y + 200 - triangle.cy),
                                     ImVec2(triangle.bx + canvas_pos.x, canvas_pos.y + 200 - triangle.by),
                                     ImVec2(triangle.ax + canvas_pos.x, canvas_pos.y + 200 - triangle.ay), triangle.color);
    }
    draw_list->PopClipRect();
    ImGui::InvisibleButton(buttonName, ImVec2(width, height));
    //
}

void frame() {
    triangles.clear();
    RenderMovieFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
    ImGui::Begin("Movie");
    ImGui::Button("What");
    Draw(320, 200, "InvButton");
    ImGui::Button("What2");
    ImGui::End();
}

int main(int, char **) {
    movie.ParseScript("movie.txt");
    movie.WriteMovie("movie.bin");
    ReadMovie("movie.bin");
    
    imgui_app(frame, "shmup Tool", 1024, 768);
    return 0;
}

#if 0
int main2(int, char**)
{
   ImApp::ImApp imApp;

   ImApp::Config config;
   config.mWidth = 1280;
   config.mHeight = 720;
   imApp.Init(config);

   printf("Started ...\n");

   Mesh mesh;

   matrix_t view, proj;

   ImGuiIO& io = ImGui::GetIO();

   float znear;
   int firstFrame = 0;
   bool expanded = true;
   int selectedEntry = -1;
   int currentFrame = 0;
   bool playing = false;
   float time = 0.f;
   int currentLevel = 0;

   std::vector<std::string> fileList;
   GetFilesList(fileList, "Levels/", ".glb", false, false, false);

   ImportGLTF(("Levels/"+ fileList[currentLevel]).c_str());
   // Main loop
   while (!imApp.Done())
   {
      imApp.NewFrame();

      currentFrame = min(currentFrame, int(gltfFrames.size() - 1));
      ConvertGLTFToMesh(gltfFrames[currentFrame], mesh, view, proj, znear);

      static const float sequencerHeight = 200;
      static const float toolWidth = 680;
      static const auto defaultWindowOptions = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;

      // 3d preview
      float previewHeight = io.DisplaySize.y - sequencerHeight;
      float previewWidth = (previewHeight / 200.f) * 320.f;
      ImGui::SetNextWindowSize(ImVec2(previewWidth, previewHeight));
      ImGui::SetNextWindowPos(ImVec2(toolWidth, 0));
      ImGui::Begin("3d", nullptr, defaultWindowOptions);
      ImVec2 displaySize = ImGui::GetContentRegionAvail();
      ImVec2 displayOffset = ImGui::GetCursorScreenPos();
      ImDrawList* drawList = ImGui::GetWindowDrawList();
      mesh.Transform(view, proj, znear);
      mesh.DebugDraw(*drawList, displaySize, displayOffset);
      bool windowHovered = ImGui::IsWindowHovered();
      ImGui::End();

      // Tool
      ImGui::SetNextWindowSize(ImVec2(toolWidth,io.DisplaySize.y - sequencerHeight));
      ImGui::SetNextWindowPos(ImVec2(0, 0));
      ImGui::Begin("Tool", nullptr, defaultWindowOptions);
      ImGui::Text(" %d Faces // %d rasterized Faces // %d colors // %d vertices", int(mesh.GetFaceCount()), int(mesh.GetRasterizedFaceCount()),
          int(mesh.frames[0].colors.size()), int(mesh.frames[0].vertices.size()));
      
      std::vector<uint32_t> debugDrawBuffer(320 * 200, 0xFFFF00FF);
      mesh.DebugDrawFrame(&mesh.frames[0], debugDrawBuffer.data());
      DrawBitmap(320, 200, debugDrawBuffer.data(), "FrameTriangles");
      DrawBitmap(320, 200, mesh.mDepthTestedColor.data(), "DepthTestedTriangles");
      ImGui::End();

      // Sequencer
      ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - sequencerHeight));
      ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, sequencerHeight));
      ImGui::Begin("Sequencer", nullptr, defaultWindowOptions);
      ImGui::PushItemWidth(300);
      if (ImGui::ListBox("Level", &currentLevel, fileList_getter, &fileList, fileList.size()))
      {
          ImportGLTF(("Levels/" + fileList[currentLevel]).c_str());
          currentFrame = 0;
      }
      ImGui::PopItemWidth();
      ImGui::SameLine();
      ImGui::BeginChildFrame(1337, ImVec2(-1,-1));
      ImGui::SliderInt("Frame", &currentFrame, 0, gltfFrames.size() - 1);
      ImGui::SameLine();
      if (ImGui::Button("|<<"))
      {
          currentFrame = 0;
      }
      ImGui::SameLine();
      if (ImGui::Button(playing ? "Stop" : "Play"))
      {
          playing = !playing;
      }

      if (ImGui::Button("Make Movie"))
      {
          std::vector<uint8_t> dump;
          int colorCount = 0;
          int faceCount = 0;
          int vertexCount = 0;
          int16_t minx = 32000;
          int16_t maxx = -32000;
          int16_t miny = 32000;
          int16_t maxy = -32000;
          mesh.frames.clear();
          for (int i = 0; i < gltfFrames.size(); i++)
          {
              ConvertGLTFToMesh(gltfFrames[i], mesh, view, proj, znear);
              mesh.Transform(view, proj, znear);
          }
          mesh.CompressColors();
          mesh.ReorderPositions();
          for (int i = 0; i < gltfFrames.size(); i++)
          {
              const auto& currentFrame = mesh.frames[i];
              auto bytes = currentFrame.GetBytes();
              colorCount += currentFrame.colors.size();
              faceCount += currentFrame.faces.size();
              vertexCount += currentFrame.vertices.size();

              for (auto& vt : currentFrame.vertices)
              {
                  minx = min(minx, vt.x);
                  maxx = max(maxx, vt.x);

                  miny = min(miny, vt.y);
                  maxy = max(maxy, vt.y);
              }
              dump.insert(dump.end(), bytes.begin(), bytes.end());
          }

          printf("%d (%d bytes) colors, %d (%d bytes) vertices, %d (%d bytes) faces.\n",
              colorCount, colorCount * sizeof(FrameColor),
              vertexCount, vertexCount * sizeof(FrameVertex),
              faceCount, faceCount * sizeof(FrameFace));

          printf("VP : %d, %d - %d, %d\n", int(minx), int(miny), int(maxx), int(maxy));

          std::string outputName = fileList[currentLevel];
          auto index = outputName.find("glb");
          if (index != std::string::npos)
          {
              outputName.replace(index, 3, "mv");
              WriteFile(("Levels/" + outputName).c_str(), dump);
          }
      }
      ImGui::EndChildFrame();
      ImGui::End();

      if (playing)
      {
          time += 1.f/60.f;
          if (time > 1.f/25.f)
          {
              time -= 1.f/25.f;
              ++currentFrame %= int(gltfFrames.size() - 1);
          }
      }

      // render everything
      glClearColor(0.45f, 0.4f, 0.4f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT);
      imApp.EndFrame();
   }

   imApp.Finish();

   return 0;
}
#endif
