#include "Movie.h"
#include <string.h>
#include <stdio.h>
#include <cstring>
#include <sstream>
#include <vector>
#include "mesh.h"
#include "gltfImport.h"
#include "moviePlayback.h"

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

bool Movie::ParseScript(const std::string& filename)
{
    FILE* fp = fopen(filename.c_str(), "rt");
	if (fp)
	{
        mSequences.clear();
        mSlots = 0;
        mBytes.clear();
        mParsingError = "";
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
            if (tmps[0] == '#')
            {
                continue;
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
                        mesh.Transform(view, proj, znear);
                    }
                    mesh.CompressColors();
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
            if (l >= 3 && tmps[0] == 'S' && tmps[1] == 'E' && tmps[2] == 'Q')
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
