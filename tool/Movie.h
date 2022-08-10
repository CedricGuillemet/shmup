#pragma once
#include <string>
#include <map>

class Movie
{
    public:
        Movie() {}

        void ParseScript(const std::string& filename);
        void WriteMovie(const std::string& filename);
    

    protected:
        void PushSequence() {}
    
        // SEQ road Levels/road.glb Cam_road 5
        struct Sequence
        {
            std::string mGLTFPath;
            std::string mCameraName;
            uint16_t mFrameCount;
            uint8_t mSlot{};
        };
    
        std::map<std::string, Sequence> mSequences;
};
