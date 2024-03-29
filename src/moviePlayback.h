#pragma once

#ifdef __cpluscplus
extern "C" {
ee
#endif

struct FrameInfos
{
	uint16_t faceCount;
	uint8_t vertexCount;
	uint8_t colorCount;
};
struct FrameVertex
{
	int16_t x, y;
};
struct FrameFace
{
	uint8_t a, b, c, colorIndex;
};
struct FrameColor
{
	uint8_t index, r, g, b;
};

inline uint32_t FrameColor32(struct FrameColor color)
{
	return (color.b << 16) + (color.g << 8) + color.r;
}

inline  struct FrameColor Color32Frame(uint32_t color)
{
	struct FrameColor frameColor;
	frameColor.r = color & 0xFF;
	frameColor.g = (color >> 8) & 0xFF;
	frameColor.b = (color >> 16) & 0xFF;
	return frameColor;
}
extern uint32_t palette[256];

#define MOVIE_SEQ 0x01
#define MOVIE_PLAY 0x02
#define MOVIE_BACKGROUND 0x03
#define MOVIE_BACKGROUND_ON 0x04
#define MOVIE_BACKGROUND_OFF 0x05
#define MOVIE_SCROLL_FROM 0x06
#define MOVIE_SCROLL_TO 0x07
#define MOVIE_SCROLL_OFF 0x08
#define MOVIE_SCROLL_ON 0x09

#define MOVIE_FOREGROUND_OFF 0x0A
#define MOVIE_FOREGROUND_ON 0x0B
#define MOVIE_LOOP 0x0C
#define MOVIE_LOOP_CLEARED 0x0D

#define MOVIE_WARP_OFF 0x0E
#define MOVIE_WARP_ON 0x0F
#define MOVIE_WARP_STRIPES_OFF 0x10
#define MOVIE_WARP_STRIPES_ON 0x11
#define MOVIE_WARP_BACKGROUND_OFF 0x12
#define MOVIE_WARP_BACKGROUND_ON 0x13

#define MOVIE_PATH 0x14
#define MOVIE_SPAWN 0x15
#define MOVIE_END 0x16
#define MOVIE_CALL 0x17
#define MOVIE_SKIP 0x18

#define MOVIE_SLOT_COUNT 32
#define MOVIE_PATH_COUNT 4096
#define MOVIE_MAX_SPAWNS 256
#define MOVIE_MAX_CALLSTACK 8
#define MOVIE_COLOR_OFFSET_BACKGROUND 200
#define MOVIE_COLOR_OFFSET_FOREGROUND 50
#define MOVIE_COLOR_COUNT_BACKGROUND 50
#define MOVIE_COLOR_COUNT_FOREGROUND 150

#define MOVIE_TO_GAMEPLAY_RATIO 2

extern int ReadMovie(const char* szPath);
extern int RenderMovieFrame();

// slow/debug
extern int GetMovieFrameCount();
extern void RenderMovieSingleFrame(int frameIndex);

unsigned char* movieData = NULL;
unsigned char* movieDataEnd = NULL;

struct MovieSpawn
{
    uint16_t pathIndex;
    int16_t x,y;
    uint16_t timeOffset;
    uint8_t type;
};

struct MovieState
{
    unsigned char* sequenceSlots[MOVIE_SLOT_COUNT];
    unsigned int sequenceSize[MOVIE_SLOT_COUNT]; // size in bytes
    unsigned char* paths[MOVIE_PATH_COUNT];
    struct MovieSpawn spawns[MOVIE_MAX_SPAWNS];
    //unsigned char* sequenceBase; //
    unsigned char* moviePointer; // global movie pointer
    unsigned char* sequencePlaying; // pointer in sequence
    unsigned char* sequencePlayingBase; // source pointer
    unsigned char* sequencePlayingEnd; // end pointer in sequence
    unsigned char playCount;
    
    // scroll
    int scrollx, scrolly;
    int scrollDeltax, scrollDeltay;
    int scrollOn;
    // state
    int foregroundDisabled;
    int frameIndex;
    // loop
    int loopCount;
    int loopDestination;
    // call stack
    unsigned char* callStack[MOVIE_MAX_CALLSTACK];
    int callStackIndex;
};

MovieState movieState;

int ReadMovie(const char* szPath)
{
    if (movieData)
    {
        free(movieData);
        movieData = NULL;
        //movieFrame = NULL;
    }
    FILE* fp = fopen(szPath, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        movieData = (unsigned char*)malloc(size);
        //movieFrame = movieData;
        movieDataEnd = movieData + size;
        fseek(fp, 0, SEEK_SET);
        fread(movieData, size, 1, fp);
        fclose(fp);
        
        // clear state
        memset(&movieState, 0, sizeof(MovieState));
        movieState.moviePointer = movieData;
        return 1;
    }
    return 0;
}

// client Overrides
void BeginBackground(unsigned short width, unsigned short height);
void StopBackground();
void BackgroundVisible(bool visible);
void DrawTriangleMovie(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy, uint8_t colorIndex);
void DrawTriangleMovieFB(uint16_t width, uint16_t height, int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy, uint8_t colorIndex);
void SetWarp(bool enabled, int frameIndex);
void SetWarpStripes(bool enabled);
void SetWarpBackground(bool enabled);
void RenderBackground(int16_t x, int16_t y);
int EnemiesCleared();
void DoSpawn(MovieSpawn* spawn);
void TickFrame();

extern uint32_t palette[256];
//int everyOtherFrame = 0;

struct FrameDecodedInfos
{
    struct FrameInfos* frameInfos;
    struct FrameVertex* frameVertices;
    struct FrameFace* frameFaces;
    struct FrameColor* frameColors;
};

void DecodeNextFrameInfos(FrameDecodedInfos* frameDecodedInfos, unsigned char** ptr)
{
    frameDecodedInfos->frameInfos = (struct FrameInfos*)*ptr;
    *ptr += sizeof(struct FrameInfos);
    frameDecodedInfos->frameVertices = (struct FrameVertex*)*ptr;
    *ptr += sizeof(struct FrameVertex) * frameDecodedInfos->frameInfos->vertexCount;
    frameDecodedInfos->frameFaces = (struct FrameFace*)*ptr;
    *ptr += sizeof(struct FrameFace) * frameDecodedInfos->frameInfos->faceCount;
    frameDecodedInfos->frameColors = (struct FrameColor*)*ptr;
    *ptr += sizeof(struct FrameColor) * frameDecodedInfos->frameInfos->colorCount;
}

void UpdatePalette(FrameDecodedInfos* frameDecodedInfos, int colorOffset);
void RenderTriangles(FrameDecodedInfos* frameDecodedInfos, int colorOffset);
void RenderTrianglesFB(uint16_t width, uint16_t height, FrameDecodedInfos* frameDecodedInfos, int colorOffset);
// return 1 if chunk found
int ReadChunk()
{
    if (movieState.moviePointer >= movieDataEnd)
    {
        return 0;
    }
    unsigned char type = *movieState.moviePointer++;
    switch(type)
    {
        case MOVIE_SEQ:
            {
                unsigned char slot = *movieState.moviePointer++;
                unsigned int size = *(unsigned int*)movieState.moviePointer;
                movieState.moviePointer += sizeof(unsigned int);
                movieState.sequenceSlots[slot] = movieState.moviePointer;
                movieState.sequenceSize[slot] = size;
                movieState.moviePointer += size;
            }
            break;
        case MOVIE_PLAY:
            {
                unsigned char slot = *movieState.moviePointer++;
                unsigned char count = *movieState.moviePointer++;
                movieState.sequencePlayingBase = movieState.sequencePlaying = movieState.sequenceSlots[slot];
                movieState.sequencePlayingEnd = movieState.sequencePlayingBase + movieState.sequenceSize[slot];
                movieState.playCount = count;
            }
            break;
        case MOVIE_BACKGROUND:
            {
                unsigned short width = *(unsigned short*)movieState.moviePointer;
                movieState.moviePointer += 2;
                unsigned short height = *(unsigned short*)movieState.moviePointer;
                movieState.moviePointer += 2;
                BeginBackground(width, height);
                
                FrameDecodedInfos frameDecodedInfos;
                DecodeNextFrameInfos(&frameDecodedInfos, &movieState.moviePointer);
                UpdatePalette(&frameDecodedInfos, MOVIE_COLOR_OFFSET_BACKGROUND);
                RenderTrianglesFB(width, height, &frameDecodedInfos, MOVIE_COLOR_OFFSET_BACKGROUND);

                StopBackground();
            }
            break;
        case MOVIE_BACKGROUND_ON:
            {
                BackgroundVisible(true);
            }
            break;
        case MOVIE_BACKGROUND_OFF:
            {
                BackgroundVisible(false);
            }
            break;
        case MOVIE_SCROLL_FROM:
            {
                movieState.scrollx = *(short*)movieState.moviePointer;
                movieState.scrollx <<= 16;
                movieState.moviePointer += 2;
                movieState.scrolly = *(short*)movieState.moviePointer;
                movieState.scrolly <<= 16;
                movieState.moviePointer += 2;
                movieState.scrollDeltax = *(unsigned int*)movieState.moviePointer;
                movieState.moviePointer += 4;
                movieState.scrollDeltay = *(unsigned int*)movieState.moviePointer;
                movieState.moviePointer += 4;
            }
            break;
        case MOVIE_SCROLL_TO:
            {
                movieState.scrollDeltax = 0;
                movieState.scrollDeltay = 0;
            }
            break;
        case MOVIE_SCROLL_ON:
            {
                movieState.scrollOn = true;
            }
            break;
        case MOVIE_SCROLL_OFF:
            {
                movieState.scrollOn = false;
            }
            break;
        case MOVIE_FOREGROUND_OFF:
            {
                movieState.foregroundDisabled = true;
            }
            break;
        case MOVIE_FOREGROUND_ON:
            {
                movieState.foregroundDisabled = false;
            }
            break;
        case MOVIE_LOOP:
            {
                if (!movieState.loopCount)
                {
                    movieState.loopDestination = *(unsigned int*)movieState.moviePointer;
                    movieState.moviePointer += 4;
                    movieState.loopCount = *movieState.moviePointer++;
                }
                // goto anyway
                movieState.loopCount --;
                if (movieState.loopCount > 0)
                {
                    movieState.moviePointer = movieData + movieState.loopDestination;
                } else {
                    // skip chunk
                    movieState.moviePointer += 5;
                }
            }
            break;
        case MOVIE_LOOP_CLEARED:
            {
                if (EnemiesCleared())
                {
                    // skip chunk
                    movieState.moviePointer += 4;
                } else {
                    movieState.moviePointer = movieData + movieState.loopDestination;
                }
            }
            break;
        case MOVIE_WARP_OFF:
            {
                SetWarp(false, 0);
            }
            break;
        case MOVIE_WARP_ON:
            {
                SetWarp(true, movieState.frameIndex);
            }
            break;
        case MOVIE_WARP_STRIPES_OFF:
            {
                SetWarpStripes(false);
            }
            break;
        case MOVIE_WARP_STRIPES_ON:
            {
                SetWarpStripes(true);
            }
            break;
        case MOVIE_WARP_BACKGROUND_OFF:
            {
                SetWarpBackground(false);
            }
            break;
        case MOVIE_WARP_BACKGROUND_ON:
            {
                SetWarpBackground(true);
            }
            break;
        case MOVIE_PATH:
            {
                uint16_t pathIndex = *(unsigned short*)movieState.moviePointer;
                movieState.moviePointer += 2;
                movieState.paths[pathIndex] = movieState.moviePointer;
                uint16_t pointCount = *(unsigned short*)movieState.moviePointer;
                movieState.moviePointer += 2;
                
                movieState.moviePointer += pointCount * 4;
            }
            break;
        case MOVIE_SPAWN:
            {
                MovieSpawn newSpawn;
                newSpawn.pathIndex = *(unsigned short*)movieState.moviePointer;
                movieState.moviePointer += 2;
                newSpawn.x = *(short*)movieState.moviePointer;
                movieState.moviePointer += 2;
                newSpawn.y = *(short*)movieState.moviePointer;
                movieState.moviePointer += 2;
                newSpawn.timeOffset = *(unsigned short*)movieState.moviePointer;
                movieState.moviePointer += 2;
                newSpawn.type = *movieState.moviePointer++;

                if (newSpawn.timeOffset == 0)
                {
                    // spawn now
                    DoSpawn(&newSpawn);
                    break;
                }
                // find a spawn entry
                for(int i = 0; i < MOVIE_MAX_SPAWNS; i++)
                {
                    struct MovieSpawn* spawn = &movieState.spawns[i];
                    if (spawn->timeOffset == 0)
                    {
                        *spawn = newSpawn;
                        break;
                    }
                }
            }
            break;
        case MOVIE_END:
            {
                movieState.callStackIndex --;
                movieState.moviePointer = movieState.callStack[movieState.callStackIndex];
            }
            break;
        case MOVIE_CALL:
            {
                uint32_t callOffset = *(uint32_t*)movieState.moviePointer;
                movieState.moviePointer += 4;
                movieState.callStack[movieState.callStackIndex++] = movieState.moviePointer;
                movieState.moviePointer = movieData + callOffset;
            }
            break;
        case MOVIE_SKIP:
            {
                uint16_t offset = *(unsigned short*)movieState.moviePointer;
                movieState.moviePointer += 2;
                movieState.moviePointer += offset;
            }
            break;
            

    }
    return 1;
}

FrameDecodedInfos DecodeNextFrame()
{
    FrameDecodedInfos frameDecodedInfos;
    frameDecodedInfos.frameInfos = NULL;
    /*if (movieFrame >= movieDataEnd)
    {
        free(movieData);
        movieData = NULL;
        return 0;
    }*/
    
    while (!movieState.playCount)
    {
        if (!ReadChunk())
        {
            return frameDecodedInfos;
        }
    };
    
    movieState.playCount --;
    movieState.frameIndex ++;
    
    // scroll
    if (movieState.scrollOn)
    {
        movieState.scrollx += movieState.scrollDeltax;
        movieState.scrolly += movieState.scrollDeltay;
    }
    
    // spawns
    for (int i = 0; i < MOVIE_MAX_SPAWNS; i++)
    {
        struct MovieSpawn* spawn = &movieState.spawns[i];
        if (spawn->timeOffset > 0)
        {
            spawn->timeOffset--;
            if (spawn->timeOffset == 0)
            {
                DoSpawn(spawn);
            }
        }
    }

    DecodeNextFrameInfos(&frameDecodedInfos, &movieState.sequencePlaying);
    // loop sequence
    if (movieState.sequencePlaying >= movieState.sequencePlayingEnd)
    {
        movieState.sequencePlaying = movieState.sequencePlayingBase;
    }
    return frameDecodedInfos;
}

void UpdatePalette(FrameDecodedInfos* frameDecodedInfos, int colorOffset)
{
    for (int i = 0; i<frameDecodedInfos->frameInfos->colorCount; i++, frameDecodedInfos->frameColors++)
    {
        palette[frameDecodedInfos->frameColors->index + colorOffset] = frameDecodedInfos->frameColors->b + (frameDecodedInfos->frameColors->g << 8) + (frameDecodedInfos->frameColors->r << 16) + 0xFF000000;
    }
}

void RenderTriangles(FrameDecodedInfos* frameDecodedInfos, int colorOffset)
{
    if (movieState.foregroundDisabled)
    {
        return;
    }
    struct FrameFace* face = frameDecodedInfos->frameFaces;
    for (int i = 0; i < frameDecodedInfos->frameInfos->faceCount; i++, face++)
    {
        struct FrameVertex vc = frameDecodedInfos->frameVertices[face->a];
        struct FrameVertex vb = frameDecodedInfos->frameVertices[face->b];
        struct FrameVertex va = frameDecodedInfos->frameVertices[face->c];
        
        DrawTriangleMovie(va.x, va.y, vb.x, vb.y, vc.x, vc.y, face->colorIndex + colorOffset);
    }
}

void RenderTrianglesFB(uint16_t width, uint16_t height, FrameDecodedInfos* frameDecodedInfos, int colorOffset)
{
    if (movieState.foregroundDisabled)
    {
        return;
    }
    struct FrameFace* face = frameDecodedInfos->frameFaces;
    for (int i = 0; i < frameDecodedInfos->frameInfos->faceCount; i++, face++)
    {
        struct FrameVertex vc = frameDecodedInfos->frameVertices[face->a];
        struct FrameVertex vb = frameDecodedInfos->frameVertices[face->b];
        struct FrameVertex va = frameDecodedInfos->frameVertices[face->c];
        
        DrawTriangleMovieFB(width, height, va.x, va.y, vb.x, vb.y, vc.x, vc.y, face->colorIndex + colorOffset);
    }
}


int RenderMovieFrame()
{
    FrameDecodedInfos frameDecodedInfos = DecodeNextFrame();
    if (!frameDecodedInfos.frameInfos)
    {
        return 0;
    }

    // set palette
    UpdatePalette(&frameDecodedInfos, MOVIE_COLOR_OFFSET_FOREGROUND);
    RenderBackground((movieState.scrollx >> 16) & 0xFFFF, (movieState.scrolly >> 16) & 0xFFFF);
    RenderTriangles(&frameDecodedInfos, MOVIE_COLOR_OFFSET_FOREGROUND);

    return 1;
}

// Debug
int GetMovieFrameCount()
{
    unsigned char* svgMoviePointer = movieState.moviePointer;
    movieState.moviePointer = movieData;

    int count = 0;
    movieState.playCount = 0;
    while (1)
    {
        while (!movieState.playCount)
        {
            if (!ReadChunk())
            {
                movieState.moviePointer = svgMoviePointer;
                return count * MOVIE_TO_GAMEPLAY_RATIO;
            }
        };
        count += movieState.playCount;
        movieState.playCount = 0;
    };
}

void RenderMovieSingleFrame(int frameIndex)
{
    memset(&movieState, 0, sizeof(MovieState));
    movieState.moviePointer = movieData;
    SetWarp(false, 0);
    SetWarpStripes(false);
    SetWarpBackground(false);
    
    for (int i = 0; i < frameIndex / MOVIE_TO_GAMEPLAY_RATIO; i++)
    {
        FrameDecodedInfos frameDecodedInfos = DecodeNextFrame();
        UpdatePalette(&frameDecodedInfos, MOVIE_COLOR_OFFSET_FOREGROUND);
        TickFrame();
    }
    RenderMovieFrame();
}

#ifdef __cpluscplus
}
#endif
