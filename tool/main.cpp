#include "imgui.h"
#define IMAPP_IMPL
#include "ImApp.h"
#include <io.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include "imgui_internal.h"
#include "mesh.h"
#include "moviePlayback.h"
#include "gltfImport.h"
#include <string>

void GetFilesList(std::vector<std::string>& aList, const char* szPath, const char* szWild, bool bRecurs, bool bDirectoriesInsteadOfFiles, bool bCompletePath)
{
    _finddata_t fileinfo;
    std::string path = szPath;
    std::string wildc = "*";

    long fret;
    std::string findDir = path + wildc;
    intptr_t fndhand = _findfirst(findDir.c_str(), &fileinfo);
    if (fndhand != -1)
    {
        do
        {
            if (strcmp(fileinfo.name, ".") && strcmp(fileinfo.name, ".."))
            {
                std::string wildc2;
                wildc2 = szPath;
                wildc2 += fileinfo.name;
                if (!(fileinfo.attrib & _A_HIDDEN))
                {
                    if (fileinfo.attrib & _A_SUBDIR)
                    {
                        if (bDirectoriesInsteadOfFiles)
                        {
                            if (bCompletePath)
                            {
                                aList.push_back(wildc2);
                            }
                            else
                            {
                                aList.push_back(fileinfo.name);
                            }
                        }

                        wildc2 += "/";
                        if (bRecurs)
                            GetFilesList(aList, wildc2.c_str(), szWild, bRecurs, bDirectoriesInsteadOfFiles, bCompletePath);
                    }
                    else
                    {
                        if (!bDirectoriesInsteadOfFiles)
                        {
                            if (strstr(wildc2.c_str(), szWild))
                            {
                                if (bCompletePath)
                                {
                                    aList.push_back(wildc2);
                                }
                                else
                                {
                                    aList.push_back(fileinfo.name);
                                }
                            }
                        }
                    }
                }
            }
            fret = _findnext(fndhand, &fileinfo);
        } 		while (fret != -1);
    }
    _findclose(fndhand);
}

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
/*
(void* data, int idx, const char** out_text) {
    *out_text = "hoho";
}*/

bool fileList_getter(void* data, int idx, const char** out_text)
{
    std::vector<std::string>* list = (std::vector<std::string>*)data;
    *out_text = (*list)[idx].c_str();
    return true;
}

int main(int, char**)
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
