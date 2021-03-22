#include "imgui.h"
#define IMAPP_IMPL
#include "ImApp.h"

#include "camera.h"
#include "Animation.h"
#include "ImSequencer.h"
#include "ImCurveEdit.h"

#include <math.h>
#include <vector>
#include <algorithm>
#include "imgui_internal.h"
#include "mesh.h"
#include "moviePlayback.h"



struct TransformEdit : public ImCurveEdit::Delegate
{
    TransformEdit(Animation& animation)
        : mAnimation(animation)
    {
        UpdateCache();
        UpdateRanges();
    }
    size_t GetCurveCount()
    {
        return 6;
    }

    bool IsVisible(size_t curveIndex)
    {
        return mbVisible[curveIndex];
    }
    size_t GetPointCount(size_t curveIndex)
    {
        return mAnimation.mKeys.size();//mPointCount[curveIndex];
    }

    uint32_t GetCurveColor(size_t curveIndex)
    {
        uint32_t cols[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
        return cols[curveIndex];
    }
    ImVec2* GetPoints(size_t curveIndex)
    {
        return mPoints[curveIndex];
    }
    virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const { return ImCurveEdit::CurveSmooth; }
    virtual int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
    {
        auto& key = mAnimation.mKeys[pointIndex];
        key.mFrame = (int)value.x;
        switch(curveIndex)
        {
        case 0:
            key.mCamera.mPosition.x = value.y;
            break;
        case 1:
            key.mCamera.mPosition.y = value.y;
            break; 
        case 2:
            key.mCamera.mPosition.z = value.y;
            break;
        case 3:
            key.mCamera.mAngles.x = value.y;
            break;
        case 4:
            key.mCamera.mAngles.y = value.y;
            break;
        case 5:
            key.mCamera.mAngles.z = value.y;
            break;
        }
        mAnimation.SortKeys();
        UpdateCache();
        return pointIndex;
    }
    virtual void AddPoint(size_t curveIndex, ImVec2 value)
    {

    }
    virtual ImVec2& GetMax() { return mMax; }
    virtual ImVec2& GetMin() { return mMin; }
    virtual unsigned int GetBackgroundColor() { return 0; }

    ImVec2 mMin;
    ImVec2 mMax;
    bool mbVisible[6];

    ImVec2 mPoints[6][1024];

    void UpdateRanges()
    {
        mMin = ImVec2(FLT_MAX, FLT_MAX);
        mMax = ImVec2(-FLT_MAX, -FLT_MAX);
        for (int i = 0; i < mAnimation.mKeys.size(); i++)
        {
            for (int j = 0; j < 6; j++)
            {
                if (mbVisible[j])
                {
                    mMin.x = (mMin.x < mPoints[j][i].x) ? mMin.x : mPoints[j][i].x;
                    mMin.y = (mMin.y < mPoints[j][i].y) ? mMin.y : mPoints[j][i].y;

                    mMax.x = (mMax.x > mPoints[j][i].x) ? mMax.x : mPoints[j][i].x;
                    mMax.y = (mMax.y > mPoints[j][i].y) ? mMax.y : mPoints[j][i].y;
                }
            }
        }
    }

    void UpdateCache()
    {
        for (int i = 0; i < mAnimation.mKeys.size(); i++)
        {
            const auto& key = mAnimation.mKeys[i];
            const float frame = (float)key.mFrame;
            mPoints[0][i] = ImVec2(frame, key.mCamera.mPosition.x);
            mPoints[1][i] = ImVec2(frame, key.mCamera.mPosition.y);
            mPoints[2][i] = ImVec2(frame, key.mCamera.mPosition.z);

            mPoints[3][i] = ImVec2(frame, key.mCamera.mAngles.x);
            mPoints[4][i] = ImVec2(frame, key.mCamera.mAngles.y);
            mPoints[5][i] = ImVec2(frame, key.mCamera.mAngles.z);
        }
    }
private:
    Animation& mAnimation;
};

struct MySequence : public ImSequencer::SequenceInterface
{
    MySequence(Animation& animation)
        : mAnimation(animation)
        , transformEdit(animation)
    {
    }
    // interface with sequencer

    virtual int GetFrameMin() const {
        return mAnimation.mStartFrame;
    }
    virtual int GetFrameMax() const {
        return mAnimation.mEndFrame;
    }
    virtual int GetItemCount() const { return 1; }

    virtual int GetItemTypeCount() const { return 1; }
    virtual const char* GetItemTypeName(int typeIndex) const { return "Camera"; }
    virtual const char* GetItemLabel(int index) const
    {
        return "Camera";
    }

    virtual void Get(int index, int** start, int** end, int* type, unsigned int* color)
    {
        if (color)
            *color = 0xFFAA8080; // same color for everyone, return color based on type
        if (start)
            *start = &mAnimation.mStartFrame;
        if (end)
            *end = &mAnimation.mEndFrame;
        if (type)
            *type = 0;
    }
    virtual void Add(int type) { };
    virtual void Del(int index) { }
    virtual void Duplicate(int index) { }

    virtual size_t GetCustomHeight(int index) { return 300; }

    TransformEdit transformEdit;

    virtual void DoubleClick(int index) {
    }
    
    virtual void CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect)
    {
        static const char* labels[] = { "x", "y", "z", "yaw", "pitch", "roll" };

        //rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
        //rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
        draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
        for (int i = 0; i < 6; i++)
        {
            ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
            ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
            draw_list->AddText(pta, transformEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
            if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
            {
                transformEdit.mbVisible[i] = !transformEdit.mbVisible[i];
            }
        }
        draw_list->PopClipRect();

        ImGui::SetCursorScreenPos(rc.Min);
        ImCurveEdit::Edit(transformEdit, ImVec2(rc.Max.x - rc.Min.x, rc.Max.y - rc.Min.y), 137 + index, &clippingRect);
    }

    virtual void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
    {
        //rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
        //rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
        /*
        draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < rampEdit.mPointCount[i]; j++)
            {
                float p = rampEdit.mPts[i][j].x;
                if (p < myItems[index].mFrameStart || p > myItems[index].mFrameEnd)
                    continue;
                float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
                float x = ImLerp(rc.Min.x, rc.Max.x, r);
                draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
            }
        }
        draw_list->PopClipRect();*/
    }

    Animation& mAnimation;
};

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

void DrawBitmap(int width, int height, uint32_t* bmp)
{
    int factor = 1;

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
    ImGui::InvisibleButton("canvas", ImVec2((float)(width * factor), (float)(height * factor)));
}

/*
playback saved movie
update view matrix when curve is edited
button 'fit' for curve fitting
save animation
level/sequence selector
error management at frame generation (# colors, # vertices, # faces)
*/

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
   //view.LookAt(vec_t(0.f, 0.f, 3.0f), vec_t(0.f, 0.f, 0.f), vec_t(0.f, 1.f, 0.f));

   Camera camera;
   Animation animation;
   MySequence sequence(animation);
   int firstFrame = 0;
   bool expanded = true;
   int selectedEntry = -1;
   int currentFrame = 0;
   // Main loop
   while (!imApp.Done())
   {
      imApp.NewFrame();

      //matrix_t viewProj = view * proj;
     
      static const float sequencerHeight = 400;
      static const float toolWidth = 340;
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
      mesh.Transform(view, proj);
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
      DrawBitmap(320, 200, debugDrawBuffer.data());
      ImGui::End();

      // Sequencer
      ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - sequencerHeight));
      ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, sequencerHeight));
      ImGui::Begin("Sequencer", nullptr, defaultWindowOptions);
      ImGui::PushItemWidth(100);
      ImGui::InputInt("Start", &animation.mStartFrame); 
      ImGui::SameLine();
      bool refreshCamera = false;
      if (ImGui::InputInt("Current", &currentFrame))
      {
          refreshCamera = true;
      }
      ImGui::SameLine();
      ImGui::InputInt("End", &animation.mEndFrame);
      ImGui::SameLine();
      if (ImGui::Button("Make Key"))
      {
          animation.MakeKey(currentFrame, camera);
          sequence.transformEdit.UpdateCache();
      }
      ImGui::SameLine();
      if (ImGui::Button("Make Movie"))
      {
          std::vector<uint8_t> dump;
          for (int i = animation.mStartFrame; i < animation.mEndFrame; i++)
          {
              Camera cam = animation.Evaluate(i);
              matrix_t camMat = cam.ComputeView();
              mesh.Transform(camMat, proj);
              auto bytes = mesh.frames[0].GetBytes();
              dump.insert(dump.end(), bytes.begin(), bytes.end());
          }
          WriteFile("D:/Dev/shmup/output.mv", dump);
      }
      ImGui::PopItemWidth();
      int previousFrame = currentFrame;
      Sequencer(&sequence, &currentFrame, &expanded, &selectedEntry, &firstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND /*| ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE*/ | ImSequencer::SEQUENCER_CHANGE_FRAME);
      refreshCamera |= currentFrame != previousFrame;
      if (refreshCamera)
      {
          camera = animation.Evaluate(currentFrame);
      }
      ImGui::End();

      float moveFactor = 0.1f;
      if (windowHovered)//!ImGui::IsAnyItemHovered() && !ImGui::IsWindowHovered() && !ImGui::IsWindowFocused())
      {
          if (io.KeysDown[io.KeyMap[ImGuiKey_LeftArrow]])
          {
              camera.mPosition -= view.right * moveFactor;
          }
          if (io.KeysDown[io.KeyMap[ImGuiKey_RightArrow]])
          {
              camera.mPosition += view.right * moveFactor;
          }
          if (io.KeysDown[io.KeyMap[ImGuiKey_UpArrow]])
          {
              camera.mPosition -= view.dir * moveFactor;
          }
          if (io.KeysDown[io.KeyMap[ImGuiKey_DownArrow]])
          {
              camera.mPosition += view.dir * moveFactor;
          }

          if (io.KeysDown[io.KeyMap[ImGuiKey_PageUp]])
          {
              camera.mAngles.z += ZPI * 0.1f;
          }

          if (io.KeysDown[io.KeyMap[ImGuiKey_PageDown]])
          {
              camera.mAngles.z -= ZPI * 0.1f;
          }

          if (io.MouseDown[0])
          {
              camera.mAngles.x -= io.MouseDelta.y * 0.003f;
              camera.mAngles.y -= io.MouseDelta.x * 0.003f;
          }

          if (io.MouseDown[2])
          {
              camera.mPosition += view.right * io.MouseDelta.x * -0.01f + view.up * io.MouseDelta.y * 0.01f;
          }
          camera.mPosition += view.dir * io.MouseWheel * -0.4f;
      }

      view = camera.ComputeView();

      // render everything
      glClearColor(0.45f, 0.4f, 0.4f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT);
      imApp.EndFrame();
   }

   imApp.Finish();

   return 0;
}
