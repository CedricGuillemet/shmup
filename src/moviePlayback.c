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
    unsigned char* sequenceSlots[8];
    unsigned int sequenceSize[8]; // size in bytes
    //unsigned char* sequenceBase; //
    unsigned char* moviePointer; // global movie pointer
    unsigned char* sequencePlaying; // pointer in sequence
    unsigned char* sequencePlayingBase; // source pointer
    unsigned char* sequencePlayingEnd; // end pointer in sequence
    unsigned char playCount;
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

void DrawTriangleMovie(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy, uint8_t colorIndex);
uint32_t palette[256];
//int everyOtherFrame = 0;

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
    }
    return 1;
}


struct FrameDecodedInfos
{
    struct FrameInfos* frameInfos;
    struct FrameVertex* frameVertices;
    struct FrameFace* frameFaces;
    struct FrameColor* frameColors;
};

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

    frameDecodedInfos.frameInfos = (struct FrameInfos*)movieState.sequencePlaying;
    movieState.sequencePlaying += sizeof(struct FrameInfos);
    frameDecodedInfos.frameVertices = (struct FrameVertex*)movieState.sequencePlaying;
    movieState.sequencePlaying += sizeof(struct FrameVertex) * frameDecodedInfos.frameInfos->vertexCount;
    frameDecodedInfos.frameFaces = (struct FrameFace*)movieState.sequencePlaying;
    movieState.sequencePlaying += sizeof(struct FrameFace) * frameDecodedInfos.frameInfos->faceCount;
    frameDecodedInfos.frameColors = (struct FrameColor*)movieState.sequencePlaying;
    movieState.sequencePlaying += sizeof(struct FrameColor) * frameDecodedInfos.frameInfos->colorCount;
        
    // loop sequence
    if (movieState.sequencePlaying >= movieState.sequencePlayingEnd)
    {
        movieState.sequencePlaying = movieState.sequencePlayingBase;
    }
    return frameDecodedInfos;
}

void UpdatePalette(FrameDecodedInfos* frameDecodedInfos)
{
    for (int i = 0; i<frameDecodedInfos->frameInfos->colorCount; i++, frameDecodedInfos->frameColors++)
    {
        palette[frameDecodedInfos->frameColors->index + 50] = frameDecodedInfos->frameColors->b + (frameDecodedInfos->frameColors->g << 8) + (frameDecodedInfos->frameColors->r << 16) + 0xFF000000;
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
    UpdatePalette(&frameDecodedInfos);

    struct FrameFace* face = frameDecodedInfos.frameFaces;
    for (int i = 0; i < frameDecodedInfos.frameInfos->faceCount; i++, face++)
    {
        struct FrameVertex vc = frameDecodedInfos.frameVertices[face->a];
        struct FrameVertex vb = frameDecodedInfos.frameVertices[face->b];
        struct FrameVertex va = frameDecodedInfos.frameVertices[face->c];
        
        DrawTriangleMovie(va.x, va.y, vb.x, vb.y, vc.x, vc.y, face->colorIndex + 50);
    }
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
    
    for (int i = 0; i < frameIndex; i++)
    {
        FrameDecodedInfos frameDecodedInfos = DecodeNextFrame();
        UpdatePalette(&frameDecodedInfos);
    }
    RenderMovieFrame();
}
