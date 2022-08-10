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
    void PushSequence(int8_t slot, const std::vector<uint8_t>& bytes);
    void PushPlayback(int8_t slot, uint8_t count);

    // SEQ road Levels/road.glb Cam_road 5
    struct Sequence
    {
        std::string mGLTFPath;
        std::string mCameraName;
        uint16_t mFrameCount;
        int8_t mSlot{-1};
    };

    std::map<std::string, Sequence> mSequences;
    
    uint8_t mSlots{0};
    int8_t AcquireSequenceSlot();
    void ReleaseSequenceSlot(int8_t slot);
};
