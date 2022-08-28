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
    void PushBackground(bool on) { mBytes.push_back(on ? MOVIE_BACKGROUND_ON : MOVIE_BACKGROUND_OFF); }
    void PushScrollOn();
    void PushScrollOff();
    void PushForeground(bool on) { mBytes.push_back(on ? MOVIE_FOREGROUND_ON : MOVIE_FOREGROUND_OFF); }
    void PushWarp(bool on) { mBytes.push_back(on ? MOVIE_WARP_ON : MOVIE_WARP_OFF); }
    void PushWarpStripes(bool on) { mBytes.push_back(on ? MOVIE_WARP_STRIPES_ON : MOVIE_WARP_STRIPES_OFF); }
    void PushWarpBackground(bool on) { mBytes.push_back(on ? MOVIE_WARP_BACKGROUND_ON : MOVIE_WARP_BACKGROUND_OFF); }
    void PushLoopCleared(uint32_t destination) { mBytes.push_back(MOVIE_LOOP_CLEARED); PushUI32(destination); };
    void PushLoop(uint32_t destination, uint8_t count) { mBytes.push_back(MOVIE_LOOP); PushUI32(destination); PushUI8(count); };
    void PushScrollFrom(int x, int y);
    void PushScrollTo(int x, int y);
    void PushPath(uint16_t pathIndex, const std::vector<Vector2>& path);
    void PushSpawn(int ship, int pathIndex, int x, int y, int timeOffset);
    void PushEnd() { mBytes.push_back(MOVIE_END); }
    void PushCall(uint32_t destination) { mBytes.push_back(MOVIE_CALL); PushUI32(destination); }
    void PushSkip(uint16_t offset) { mBytes.push_back(MOVIE_SKIP); PushUI16(offset); }
    
    void PushUI32(uint32_t v);
    void PushI32(uint32_t v);
    void PushUI16(uint16_t v);
    void PushI16(int16_t v);
    void PushUI8(uint8_t v) { mBytes.push_back(v); }
   
    
    bool ParsePoint(const std::string& str, int line, int& x, int& y);
    // SEQ road Levels/road.glb Cam_road 5
    struct Sequence
    {
        std::string mGLTFPath;
        std::string mCameraName;
        uint16_t mFrameCount;
        int8_t mSlot{-1};
    };
    
    struct Path
    {
        uint16_t pathIndex;
        std::vector<Vector2> points;
    };

    std::map<std::string, Sequence> mSequences;
    std::map<std::string, uint32_t> mLabels;
    std::map<std::string, uint32_t> mFunctions;
    std::map<std::string, Path> mPaths;
    uint32_t mSlots{0};
    std::vector<uint8_t> mBytes;
    std::string mParsingError;
    int mScrollDeltaIndex{};
    int mScrollFrameCount;
    int mFromx, mFromy;
    bool mScrollOn{};
    bool mIsInDefinition{};
    uint32_t mSkipOffset;
        
    
    int8_t AcquireSequenceSlot();
    bool ReleaseSequenceSlot(int8_t slot);

};


static int ParseTokens(char* str, std::vector<std::string>& strings)
{
    /*static const char* sep = " ";
    
    char* token = strtok(str, sep);
    int index = 0;
    while(token != NULL)
    {
        strings.push_back(token);
        index++;
        token = strtok(NULL, sep);
    }
    return index;*/
    
    strings.clear();
    const int l = strlen(str);
    bool inParenthesis = false;
    std::string currentString;
    for (int i = 0; i < l; i++)
    {
        char chr = str[i];
        if (chr == '(')
        {
            if (currentString.size()>0)
            {
                strings.push_back(currentString);
                currentString = "";
            }
            inParenthesis = true;
        }
        else if (chr == ')')
        {
            if (currentString.size()>0)
            {
                strings.push_back(currentString);
                currentString = "";
            }
            inParenthesis = false;
        }
        else if (((chr == ' ' || chr == '\t') && !inParenthesis) || (chr == '\n') )
        {
            if (currentString.size()>0)
            {
                strings.push_back(currentString);
                currentString = "";
            }
        }
        else
        {
            currentString += chr;
        }
    }
    if (currentString.size()>0)
    {
        strings.push_back(currentString);
    }
    return strings.size();
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

void Movie::PushPath(uint16_t pathIndex, const std::vector<Vector2>& path)
{
    mBytes.push_back(MOVIE_PATH);
    PushUI16(pathIndex);
    PushUI16((uint16_t)path.size());
    for(auto v : path)
    {
        PushI16(v.x.parts.integer);
        PushI16(v.y.parts.integer);
    }
}

bool Movie::ParsePoint(const std::string& str, int line, int& x, int& y)
{
    if (sscanf(str.c_str(), "%d,%d", &x, &y) != 2)
    {
        std::stringstream strm;
        strm << "Unable to parse path point line " << line;
        mParsingError = strm.str();
        return false;
    }
    return true;
}

void Movie::PushSpawn(int ship, int pathIndex, int x, int y, int timeOffset)
{
    mBytes.push_back(MOVIE_SPAWN);
    PushUI16(pathIndex);
    PushI16(x);
    PushI16(y);
    PushUI16(timeOffset);
    PushUI8((uint8_t)ship);
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
        mLabels.clear();
        mPaths.clear();
        mIsInDefinition = false;
        mFunctions.clear();

        int line = 0;

        std::vector<std::string> strings;
        while(!feof(fp))
        {
            line++;
            char tmps[1024];
            fgets(tmps, sizeof(tmps), fp);
            
            int tokenCount = ParseTokens(tmps, strings);
            if (!tokenCount || strings[0][0] == '#')
            {
                continue;
            }
            // END
            else if (strings[0] == "END")
            {
                if (mIsInDefinition)
                {
                    mIsInDefinition = false;
                    PushEnd();
                    uint16_t offset = mBytes.size() - (mSkipOffset + 3); // 3 == size of chunk skip
                    uint16_t* offsetPtr = (uint16_t*)&mBytes[mSkipOffset + 1];
                    *offsetPtr = offset;
                } else {
                    break;
                }
            }
            else if (strings[0] == "CALL")
            {
                const auto& functionName = strings[1];
                auto iter = mFunctions.find(functionName);
                if (iter == mFunctions.end())
                {
                    std::stringstream strm;
                    strm << "Trying to call an undefined function " << line;
                    mParsingError = strm.str();
                    return false;
                }
                PushCall(iter->second);
            }
            else if (strings[0] == "FUNCTION")
            {
                if (mIsInDefinition)
                {
                    std::stringstream strm;
                    strm << "Nested function definition not allowed " << line;
                    mParsingError = strm.str();
                    return false;
                }
                const auto& functionName = strings[1];
                auto iter = mFunctions.find(functionName);
                if (iter != mFunctions.end())
                {
                    std::stringstream strm;
                    strm << "Function name if already existing " << line;
                    mParsingError = strm.str();
                    return false;
                }
                
                mSkipOffset = mBytes.size();
                PushSkip(0);
                mFunctions[functionName] = mBytes.size();
                mIsInDefinition = true;
            }
            // SPAWN
            else if (strings[0] == "SPAWN")
            {
                // SPAWN WhiteHunter p0 (330, 80) 32
                const std::string& pathName = strings[2];
                auto iter = mPaths.find(pathName);
                if (iter == mPaths.end())
                {
                    std::stringstream strm;
                    strm << "Path not found line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                
                int x, y;
                if (!ParsePoint(strings[3], line, x, y))
                {
                    return false;
                }
                int timeOffset = atoi(strings[4].c_str());
                EnemyType enemyType = EnemyCount;
                struct EnemyTypes
                {
                    EnemyType type;
                    const char* str;
                };
                static EnemyTypes types[EnemyCount] = {
                    {EnemyTypeWhite, "White"},
                    {EnemyTypeBlack, "Black"},
                    {EnemyTypeWhiteSmall, "WhiteSmall"},
                    {EnemyTypeBlackSmall, "BlackSmall"},
                    {EnemyTypeWhiteHunter, "WhiteHunter"},
                    {EnemyTypeBlackHunter, "BlackHunter"},
                    {EnemyTypeWhiteBig, "WhiteBig"},
                    {EnemyTypeBlackBig, "BlackBig"}};
                for (auto& type : types)
                {
                    if (strings[1] == type.str)
                    {
                        enemyType = type.type;
                        break;
                    }
                }
                
                if (enemyType == EnemyCount)
                {
                    std::stringstream strm;
                    strm << "Unknown enemy type line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                
                PushSpawn(enemyType, iter->second.pathIndex, x, y, timeOffset);
            }
            // PATH
            else if (strings[0] == "PATH")
            {
                if (tokenCount <= 2)
                {
                    std::stringstream strm;
                    strm << "Syntax error line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                const std::string& pathName = strings[1];
                auto iter = mPaths.find(pathName);
                if (iter != mPaths.end())
                {
                    std::stringstream strm;
                    strm << "Path name already exists line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                std::vector<Vector2> path;
                for (int pointIndex = 0; pointIndex < tokenCount - 2; pointIndex++)
                {
                    int x,y;
                    if (!ParsePoint(strings[pointIndex + 2], line, x, y))
                    {
                        return false;
                    }
                    path.push_back(V2FromInt(x, y));
                }
                uint16_t pathIndex = (uint16_t)mPaths.size();
                mPaths[pathName] = Path{pathIndex, path};
                PushPath(pathIndex, path);
            }
            // LOOP
            else if (strings[0] == "LOOP")
            {
                if (tokenCount != 3)
                {
                    std::stringstream strm;
                    strm << "Syntax error line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                auto iter = mLabels.find(strings[2]);
                if (iter == mLabels.end())
                {
                    std::stringstream strm;
                    strm << "Label " << strings[2] << " not defined Line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                if (strings[1] == "CLEARED")
                {
                    PushLoopCleared(iter->second);
                }
                int loopCount = atoi(strings[1].c_str());
                PushLoop(iter->second, loopCount);
            }
            // LABEL
            else if (strings[0] == "LABEL")
            {
                auto iter = mLabels.find(strings[1]);
                if (iter != mLabels.end())
                {
                    std::stringstream strm;
                    strm << "Label " << strings[1] << " already defined Line " << line;
                    mParsingError = strm.str();
                    return false;
                }
                mLabels[strings[1]] = mBytes.size();
            }
            // WARP
            else if (strings[0] == "WARP")
            {
                bool isOn = (tokenCount == 2 && strings[1].substr(0, 2) == "ON") || (tokenCount == 3 && strings[2].substr(0, 2) == "ON");
                
                if (tokenCount == 2)
                {
                    PushWarp(isOn);
                } else if (tokenCount == 3)
                {
                    if (strings[1] == "BACKGROUND")
                    {
                        PushWarpBackground(isOn);
                    } else if (strings[1] == "STRIPES")
                    {
                        PushWarpStripes(isOn);
                    }
                }
                
            }
            // foreground
            else if (strings[0] == "FORE")
            {
                PushForeground(strings[1] == "ON");
            }
            // BACK ON
            else if (strings[0] == "BACK")
            {
                if (strings[1] == "ON" || strings[1] == "OFF")
                {
                    PushBackground(strings[1] == "ON");
                    continue;
                }
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
            else if (strings[0] == "PLAY")
            {
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
            else if (strings[0] == "SEQ")
            {
                Sequence seq;
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
        
        if (mIsInDefinition)
        {
            std::stringstream strm;
            strm << "Script ends in a function definition";
            mParsingError = strm.str();
            return false;
        }
        
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
