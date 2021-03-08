#include "imgui.h"
#define IMAPP_IMPL
#include "ImApp.h"

/*
#include "ImGuizmo.h"
#include "ImSequencer.h"
#include "ImZoomSlider.h"
*/
#include <math.h>
#include <vector>
#include <algorithm>
/*
#include "ImCurveEdit.h"
*/
#include "imgui_internal.h"

#include "mesh.h"
//
//
// ImGuizmo example helper functions
//
//

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


int main(int, char**)
{
   ImApp::ImApp imApp;

   ImApp::Config config;
   config.mWidth = 1280;
   config.mHeight = 720;
   //config.mFullscreen = true;
   imApp.Init(config);

   int lastUsing = 0;
   

   Mesh mesh;
   mesh.LoadObj("", "");
   mesh.ApplyDirectional();

   matrix_t view, proj;

   ImGuiIO& io = ImGui::GetIO();

   float fov = 27.f;
   proj.glhPerspectivef2(fov, io.DisplaySize.x / io.DisplaySize.y, 0.1f, 1000.f);
   view.lookAtRH(vec_t(4.f, 4.f, 4.f), vec_t(0.f, 0.f, 0.f), vec_t(0.f, 1.f, 0.f));

   matrix_t viewProj = view * proj;

   // Main loop
   while (!imApp.Done())
   {
      imApp.NewFrame();

      mesh.Transform(view, proj);
      auto drawList = BeginFrame();
      mesh.DebugDraw(*drawList);
      
      std::vector<uint32_t> debugDrawBuffer(320 * 200, 0xFFFF00FF);
      mesh.DebugDrawFrame(&mesh.frames[0], debugDrawBuffer.data());

      ImGui::Begin("shmup");
      ImGui::Text(" %d Faces", int(mesh.GetFaceCount()));
      ImGui::Text(" %d visible Faces", int(mesh.GetTransformedFaceCount()));
      ImGui::Text(" %d rasterized Faces", int(mesh.GetRasterizedFaceCount()));
      

      ImGui::End();

      // render everything
      glClearColor(0.45f, 0.4f, 0.4f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT);
      imApp.EndFrame();
   }

   imApp.Finish();

   return 0;
}