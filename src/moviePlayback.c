#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "moviePlayback.h"
#include <assert.h>
unsigned char* movieData = NULL;
unsigned char* movieDataEnd = NULL;
//unsigned char* movieFrame = NULL;
//unsigned char* moviePointer = NULL;

struct MovieState
{
    unsigned char* sequenceSlots[MOVIE_SLOT_COUNT];
    unsigned int sequenceSize[MOVIE_SLOT_COUNT]; // size in bytes
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
extern void BeginBackground(unsigned short width, unsigned short height);
extern void StopBackground();
extern void BackgroundVisible(bool visible);
extern void SetScroll(int16_t x, int16_t y);

void DrawTriangleMovie(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy, uint8_t colorIndex);
uint32_t palette[256];
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
                RenderTriangles(&frameDecodedInfos, MOVIE_COLOR_OFFSET_BACKGROUND);

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
                movieState.moviePointer += 2;
                movieState.scrollDeltay = *(unsigned int*)movieState.moviePointer;
                movieState.moviePointer += 2;
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
    
    // scroll
    if (movieState.scrollOn)
    {
        movieState.scrollx += movieState.scrollDeltax;
        movieState.scrolly += movieState.scrollDeltay;
    }
    SetScroll((movieState.scrollx >> 16) & 0xFFFF, (movieState.scrolly >> 16) & 0xFFFF);

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
    struct FrameFace* face = frameDecodedInfos->frameFaces;
    for (int i = 0; i < frameDecodedInfos->frameInfos->faceCount; i++, face++)
    {
        struct FrameVertex vc = frameDecodedInfos->frameVertices[face->a];
        struct FrameVertex vb = frameDecodedInfos->frameVertices[face->b];
        struct FrameVertex va = frameDecodedInfos->frameVertices[face->c];
        
        DrawTriangleMovie(va.x, va.y, vb.x, vb.y, vc.x, vc.y, face->colorIndex + colorOffset);
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
                return count;
            }
        };
        count += movieState.playCount;
        movieState.playCount = 0;
    };
}

void RenderMovieFrame(int frameIndex)
{
    movieState.moviePointer = movieData;
    movieState.playCount = 0;
    
    for (int i = 0; i < frameIndex; i++)
    {
        FrameDecodedInfos frameDecodedInfos = DecodeNextFrame();
        UpdatePalette(&frameDecodedInfos, MOVIE_COLOR_OFFSET_FOREGROUND);
    }
    RenderMovieFrame();
}
