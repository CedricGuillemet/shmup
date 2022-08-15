#pragma once
#include <string>
#include <map>
#include <vector>

class Movie
{
public:
    Movie() {}

    // return true if OK
    bool ParseScript(const std::string& filename);
    void WriteMovie(const std::string& filename);

    const std::string& GetParsingError() const { return mParsingError; }
protected:
    void PushSequence(int8_t slot, const std::vector<uint8_t>& bytes);
    void PushPlayback(int8_t slot, uint8_t count);
    void PushUI32(uint32_t v);

    // SEQ road Levels/road.glb Cam_road 5
    struct Sequence
    {
        std::string mGLTFPath;
        std::string mCameraName;
        uint16_t mFrameCount;
        int8_t mSlot{-1};
    };

    std::map<std::string, Sequence> mSequences;
    uint32_t mSlots{0};
    std::vector<uint8_t> mBytes;
    std::string mParsingError;
    
    int8_t AcquireSequenceSlot();
    bool ReleaseSequenceSlot(int8_t slot);

};
