#pragma once
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include "gltfImport.h"
#include "mesh.h"
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
    void PushBackground(uint16_t width, uint16_t height, const std::vector<uint8_t>& bytes);
    void PushBackgroundOn();
    void PushBackgroundOff();
    void PushScrollOn();
    void PushScrollOff();
    void PushScrollFrom(int x, int y);
    void PushScrollTo(int x, int y);
    void PushUI32(uint32_t v);
    void PushI32(uint32_t v);
    void PushUI16(uint16_t v);
    void PushI16(int16_t v);

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
    int mScrollDeltaIndex{};
    int mScrollFrameCount;
    int mFromx, mFromy;
    bool mScrollOn{};
    
    int8_t AcquireSequenceSlot();
    bool ReleaseSequenceSlot(int8_t slot);

};

static int ParseTokens(char* str, std::vector<std::string>& strings)
{
    static const char* sep = " ";
    
    char* token = strtok(str, sep);
    int index = 0;
    while(token != NULL)
    {
        strings.push_back(token);
        index++;
        token = strtok(NULL, sep);
    }
    return index;
}

static void ConvertGLTFToMesh(const GLTFFrame& frame, Mesh& mesh, Imm::matrix& view, Imm::matrix& projection, float& znear)
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

int8_t Movie::AcquireSequenceSlot()
{
    for (int i = 0; i < MOVIE_SLOT_COUNT; i++)
    {
        int slot = 1 << i;
        if (!(mSlots & slot))
        {
            mSlots |= slot;
            return i;
        }
    }
    // ERROR MESSAGE : no free slot
    return -1;
}

bool Movie::ReleaseSequenceSlot(int8_t slot)
{
    if (slot < 0 || slot >= 8 || !(mSlots & (1<<slot)))
    {
        return false;
    }
    mSlots &= ~(1<<slot);
    return true;
}

void Movie::PushUI32(uint32_t v)
{
    uint8_t* val = (uint8_t*)&v;
    for (int i = 0; i < 4; i++)
    {
        mBytes.push_back(*val++);
    }
}

void Movie::PushI32(uint32_t v)
{
    uint8_t* val = (uint8_t*)&v;
    for (int i = 0; i < 4; i++)
    {
        mBytes.push_back(*val++);
    }
}

void Movie::PushUI16(uint16_t v)
{
    uint8_t* val = (uint8_t*)&v;
    for (int i = 0; i < 2; i++)
    {
        mBytes.push_back(*val++);
    }
}

void Movie::PushI16(int16_t v)
{
    uint8_t* val = (uint8_t*)&v;
    for (int i = 0; i < 2; i++)
    {
        mBytes.push_back(*val++);
    }
}

void Movie::PushSequence(int8_t slot, const std::vector<uint8_t>& bytes)
{
    mBytes.push_back(MOVIE_SEQ);
    mBytes.push_back((uint8_t)slot);
    PushUI32(uint32_t(bytes.size()));
    mBytes.insert(mBytes.end(), bytes.begin(), bytes.end());
}

void Movie::PushPlayback(int8_t slot, uint8_t count)
{
    mBytes.push_back(MOVIE_PLAY);
    mBytes.push_back((uint8_t)slot);
    mBytes.push_back(count);
}

void Movie::PushBackground(uint16_t width, uint16_t height, const std::vector<uint8_t>& bytes)
{
    mBytes.push_back(MOVIE_BACKGROUND);
    PushUI16(width);
    PushUI16(height);
    mBytes.insert(mBytes.end(), bytes.begin(), bytes.end());
}

void Movie::PushBackgroundOn()
{
    mBytes.push_back(MOVIE_BACKGROUND_ON);
}

void Movie::PushBackgroundOff()
{
    mBytes.push_back(MOVIE_BACKGROUND_OFF);
}

void Movie::PushScrollFrom(int x, int y)
{
    mBytes.push_back(MOVIE_SCROLL_FROM);
    PushI16((int16_t)x);
    PushI16((int16_t)y);
    
    mFromx = x;
    mFromy = y;
    
    // deltas
    mScrollDeltaIndex = mBytes.size();
    PushI32(0);
    PushI32(0);
    mScrollFrameCount = 0;
}

void Movie::PushScrollTo(int x, int y)
{
    mBytes.push_back(MOVIE_SCROLL_TO);
    
    int dx = x - mFromx;
    int dy = y - mFromy;
    dx *= 0x10000;
    dy *= 0x10000;
    dx /= mScrollFrameCount;
    dy /= mScrollFrameCount;
    
    *(int32_t*)&mBytes[mScrollDeltaIndex] = dx;
    *(int32_t*)&mBytes[mScrollDeltaIndex+4] = dy;
    mScrollDeltaIndex = 0;
}

void Movie::PushScrollOn()
{
    mBytes.push_back(MOVIE_SCROLL_ON);
    mScrollOn = true;
}

void Movie::PushScrollOff()
{
    mBytes.push_back(MOVIE_SCROLL_OFF);
    mScrollOn = false;
}

bool Movie::ParseScript(const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "rt");
    if (fp)
    {
        mSequences.clear();
        mSlots = 0;
        mBytes.clear();
        mParsingError = "";
        mScrollDeltaIndex = 0;
        mScrollFrameCount = 0;
        mFromx = 0;
        mFromy = 0;
        mScrollOn = false;

        int line = 0;

        while(!feof(fp))
        {
            line++;
            char tmps[1024];
            fgets(tmps, sizeof(tmps), fp);
            auto l = strlen(tmps);
            if (!l)
            {
                continue;
            }
            else if (l >= 3 && tmps[0] == 'E' && tmps[1] == 'N' && tmps[2] == 'D')
            {
                break;
            }
            else if (tmps[0] == '#')
            {
                continue;
            }
            // BACK ON
            else if (l >= 7 && tmps[0] == 'B' && tmps[1] == 'A' && tmps[2] == 'C' && tmps[3] == 'K' && tmps[5] == 'O' && tmps[6] == 'N')
            {
                PushBackgroundOn();
            }
            // BACK OFF
            else if (l >= 8 && tmps[0] == 'B' && tmps[1] == 'A' && tmps[2] == 'C' && tmps[3] == 'K' && tmps[5] == 'O' && tmps[6] == 'F' && tmps[7] == 'F')
            {
                PushBackgroundOff();
            }

            // BACK Levels/road_back.glb Cam_background 640 100
            else if (l >= 4 && tmps[0] == 'B' && tmps[1] == 'A' && tmps[2] == 'C' && tmps[3] == 'K')
            {
                std::vector<std::string> strings;
                int tokenCount = ParseTokens(tmps, strings);
                if (tokenCount != 5 && tokenCount != 3)
                {
                    std::stringstream strm;
                    strm << "Invalid syntax Line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                
                // scroll
                if (strings[1] == "SCROLL")
                {
                    if (strings[2].substr(0, 2) == "ON")
                    {
                        PushScrollOn();
                    }
                    else if (strings[2].substr(0, 3) == "OFF")
                    {
                        PushScrollOff();
                    }
                    else if (strings[2] == "FROM" || strings[2] == "TO")
                    {
                        // BACK SCROLL TO 320 0
                        bool from = strings[2] == "FROM";
                        int x = atoi(strings[3].c_str());
                        int y = atoi(strings[4].c_str());
                        
                        if (from)
                        {
                            PushScrollFrom(x, y);
                        }
                        else
                        {
                            // TO
                            if (!mScrollDeltaIndex)
                            {
                                std::stringstream strm;
                                strm << "Scroll has a 'TO' without a 'FROM' Line " << line;
                                mParsingError = strm.str();
                                return false;
                            }
                            PushScrollTo(x, y);
                        }
                    }
                    else
                    {
                        std::stringstream strm;
                        strm << "Invalid scroll command Line " << line;
                        mParsingError = strm.str();
                        return false;

                    }
                }
                else
                {
                    // BACKGROUND definition
                    const std::string& gltfPath = strings[1];
                    const std::string& camera = strings[2];
                    const int width = atoi(strings[3].c_str());
                    const int height = atoi(strings[4].c_str());
                    
                    Imm::matrix view, proj;
                    float znear;
                    
                    auto gltfFrames = ImportGLTF(gltfPath.c_str(), camera.c_str());
                    Mesh mesh;
                    std::vector<uint8_t> dump;
                    if (gltfFrames.size() != 1)
                    {
                        std::stringstream strm;
                        strm << "Background has more than 1 frame Line " << line;
                        mParsingError = strm.str();
                        return false;
                    }

                    ConvertGLTFToMesh(gltfFrames[0], mesh, view, proj, znear);
                    mesh.Transform(view, proj, znear, width, height);
                    if (!mesh.CompressColors(MOVIE_COLOR_COUNT_BACKGROUND))
                    {
                        std::stringstream strm;
                        strm << "Too many colors Background " << gltfPath.c_str() << " with camera " << camera.c_str();
                        mParsingError = strm.str();
                        return false;
                    }
                    const auto& currentFrame = mesh.frames[0];
                    auto bytes = currentFrame.GetBytes();
                    dump.insert(dump.end(), bytes.begin(), bytes.end());
                    PushBackground(width, height, dump);
                }
            }
            // PLAY
            if (l >= 4 && tmps[0] == 'P' && tmps[1] == 'L' && tmps[2] == 'A' && tmps[3] == 'Y')
            {
                std::vector<std::string> strings;
                int tokenCount = ParseTokens(tmps, strings);
                if (tokenCount != 2 && tokenCount != 3)
                {
                    std::stringstream strm;
                    strm << "Invalid syntax Line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                int playCount = (tokenCount == 3) ? atoi(strings[2].c_str()) : 1;
                if (mScrollOn)
                {
                    mScrollFrameCount += playCount;
                }

                auto iter = mSequences.find(strings[1]);
                if (iter == mSequences.end())
                {
                    std::stringstream strm;
                    strm << "Sequence not found Line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                // sequence found
                auto& seq = iter->second;
                if (seq.mSlot < 0)
                {
                    // not parsed yet
                    Imm::matrix view, proj;
                    float znear;
                    
                    auto gltfFrames = ImportGLTF(seq.mGLTFPath.c_str(), seq.mCameraName.c_str());
                    Mesh mesh;
                    std::vector<uint8_t> dump;
                    auto frameCount{seq.mFrameCount};
                    if (!gltfFrames.size())
                    {
                        std::stringstream strm;
                        strm << "GLTF not found or has no frames line " << line;
                        mParsingError = strm.str();
                        return false;
                    }
                    if (frameCount > gltfFrames.size())
                    {
                        std::stringstream strm;
                        strm << "Not enough animation frames between camera and sequence line " << line;
                        mParsingError = strm.str();
                        return false;
                    }
                    for (int i = 0; i < frameCount; i++)
                    {
                        ConvertGLTFToMesh(gltfFrames[i], mesh, view, proj, znear);
                        mesh.Transform(view, proj, znear, 320, 200);
                    }
                    if (!mesh.CompressColors(MOVIE_COLOR_COUNT_FOREGROUND))
                    {
                        std::stringstream strm;
                        strm << "Too many colors Sequence " << seq.mGLTFPath.c_str() << " with camera " << seq.mCameraName.c_str();
                        mParsingError = strm.str();
                        return false;
                    }
                    //mesh.ReorderPositions();
                    for (int i = 0; i < frameCount; i++)
                    {
                        const auto& currentFrame = mesh.frames[i];
                        auto bytes = currentFrame.GetBytes();
                        dump.insert(dump.end(), bytes.begin(), bytes.end());
                    }
                    seq.mSlot = AcquireSequenceSlot();
                    if (seq.mSlot < 0)
                    {
                        std::stringstream strm;
                        strm << "Unable to find a free slot for sequence at line " << line << " Consider releasing using less sequences at once.";
                        mParsingError = strm.str();
                        return false;
                    }
                    PushSequence(seq.mSlot, dump);
                }
                
                PushPlayback(seq.mSlot, playCount);
            }
            // SEQ
            else if (l >= 3 && tmps[0] == 'S' && tmps[1] == 'E' && tmps[2] == 'Q')
            {
                Sequence seq;
                
                std::vector<std::string> strings;
                int tokenCount = ParseTokens(tmps, strings);
                if (tokenCount != 5)
                {
                    std::stringstream strm;
                    strm << "Invalid syntax Line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                std::string name = strings[1];
                seq.mGLTFPath = strings[2];
                seq.mCameraName = strings[3];
                seq.mFrameCount = atoi(strings[4].c_str());
                
                mSequences.insert(std::make_pair(name, seq));
            }
        } // while !feof
        
        fclose(fp);
        return true;
    }
    mParsingError = "Unable to open movie text file.";
    return false;
}

void Movie::WriteMovie(const std::string& filename)
{
    WriteFile(filename.c_str(), mBytes);
}
