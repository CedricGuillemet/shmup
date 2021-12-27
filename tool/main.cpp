#include "imgui.h"
#define IMAPP_IMPL
#include "ImApp.h"

#include "camera.h"
#include "Animation.h"

#include <math.h>
#include <vector>
#include <algorithm>
#include "imgui_internal.h"
#include "mesh.h"
#include "moviePlayback.h"
#include "gltfImport.h"

ImDrawList* BeginFrame()
{
    const ImU32 flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus;

#ifdef IMGUI_HAS_VIEWPORT
    ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->Pos);
#else
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::SetNextWindowPos(ImVec2(0, 0));
#endif

    ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    ImGui::Begin("gizmo", NULL, flags);
    ImDrawList* res = ImGui::GetWindowDrawList();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    return res;
}

void DrawBitmap(int width, int height, uint32_t* bmp, const char* buttonName)
{
    int factor = 2;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            ImVec2 st = canvas_pos;
            st.x += x * factor;
            st.y +=  y * factor;
            draw_list->AddRectFilled(st, ImVec2(st.x + factor, st.y + factor), bmp[y * width + x]);
        }
    }
    ImGui::InvisibleButton(buttonName, ImVec2((float)(width * factor), (float)(height * factor)));
}

void ConvertGLTFToMesh(const GLTFFrame& frame, Mesh& mesh, matrix_t& view, matrix_t& projection, float& znear)
{
    znear = frame.znear;
    mesh.mPositions.resize(frame.positions.size());
    for(uint32_t i = 0; i < frame.positions.size(); i++)
    {
        mesh.mPositions[i].x = frame.positions[i].x;
        mesh.mPositions[i].y = frame.positions[i].y;
        mesh.mPositions[i].z = frame.positions[i].z;
    }

    mesh.mFaces.resize(frame.triangles.size());
    for (uint32_t i = 0; i < frame.triangles.size(); i++)
    {
        auto& f = mesh.mFaces[i];
        auto& tri = frame.triangles[i];
        f.a = tri.a;
        f.b = tri.b;
        f.c = tri.c;
        f.mColor.SetVect({tri.red, tri.green, tri.blue});
    }

    memcpy(&view, frame.view.m, sizeof(float) * 16);
    memcpy(&projection, frame.projection.m, sizeof(float) * 16);
}

int main(int, char**)
{
   ImApp::ImApp imApp;

   ImApp::Config config;
   config.mWidth = 1280;
   config.mHeight = 720;
   //config.mFullscreen = true;
   imApp.Init(config);

   //InitFBX("e:/factory_pipe_cam.fbx");
   ImportGLTF("e:/factory_pipe.glb");
   //ImportGLTF("e:/factory_pipe_ground.glb");
   //ImportGLTF("e:/clipit.glb");

   Mesh mesh;
   

   matrix_t view, proj;

   ImGuiIO& io = ImGui::GetIO();

   float znear;
   //float fov = 27.f;
   //proj.glhPerspectivef2(fov, io.DisplaySize.x / io.DisplaySize.y, 0.1f, 1000.f);
   //view.LookAt(vec_t(0.f, 0.f, 3.0f), vec_t(0.f, 0.f, 0.f), vec_t(0.f, 1.f, 0.f));

   Camera camera;
   Animation animation;
   int firstFrame = 0;
   bool expanded = true;
   int selectedEntry = -1;
   int currentFrame = 0;//703; // 24
   bool playing = false;
   float time = 0.f;
   // Main loop
   while (!imApp.Done())
   {
      imApp.NewFrame();

      
      //matrix_t viewProj = view * proj;

      currentFrame = min(currentFrame, int(gltfFrames.size() - 1));
      ConvertGLTFToMesh(gltfFrames[currentFrame], mesh, view, proj, znear);
      //mesh.ApplyDirectional();

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
      ImGui::Text(" %d Faces // %d rasterized Faces", int(mesh.GetFaceCount()), int(mesh.GetRasterizedFaceCount()));
      
      std::vector<uint32_t> debugDrawBuffer(320 * 200, 0xFFFF00FF);
      mesh.DebugDrawFrame(&mesh.frames[0], debugDrawBuffer.data());
      DrawBitmap(320, 200, debugDrawBuffer.data(), "FrameTriangles");
      DrawBitmap(320, 200, mesh.mDepthTestedColor.data(), "DepthTestedTriangles");
      ImGui::End();

      // Sequencer
      ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - sequencerHeight));
      ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, sequencerHeight));
      ImGui::Begin("Sequencer", nullptr, defaultWindowOptions);
      ImGui::SliderInt("Frame", &currentFrame, 0, gltfFrames.size() - 1);
      if (ImGui::Button("Make Movie"))
      {
          std::vector<uint8_t> dump;
          for (int i = animation.mStartFrame; i < animation.mEndFrame; i++)
          {
              ConvertGLTFToMesh(gltfFrames[i], mesh, view, proj, znear);
              mesh.Transform(view, proj, znear);
              auto bytes = mesh.frames[0].GetBytes();
              dump.insert(dump.end(), bytes.begin(), bytes.end());
          }
          WriteFile("D:/Dev/shmup/output.mv", dump);
      }
      ImGui::SameLine();
      if (ImGui::Button("|<<"))
      {
          currentFrame = 0;
      }
      if (ImGui::Button(playing ? "Stop" : "Play"))
      {
          playing = !playing;
      }
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
