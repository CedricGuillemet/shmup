#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include "moviePlayback.h"

unsigned char* movieData = NULL;
unsigned char* movieDataEnd = NULL;
unsigned char* movieFrame = NULL;


int ReadMovie(const char* szPath)
{
	if (movieData)
	{
		free(movieData);
		movieData = NULL;
		movieFrame = NULL;
	}
	FILE* fp = fopen(szPath, "rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		movieData = (unsigned char*)malloc(size);
		movieFrame = movieData;
		movieDataEnd = movieData + size;
		fseek(fp, 0, SEEK_SET);
		fread(movieData, size, 1, fp);
		fclose(fp);
		return 1;
	}
	return 0;
}

void DrawTriangleMovie(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy, uint8_t colorIndex);
extern uint32_t palette[256];
int everyOtherFrame = 0;
int RenderMovieFrame()
{
	if (movieFrame >= movieDataEnd)
	{
		free(movieData);
		movieData = NULL;
		return 0;
	}

	uint8_t* currentFrame = movieFrame;

	struct FrameInfos* frameInfos = (struct FrameInfos*)movieFrame;
	movieFrame += sizeof(struct FrameInfos);
	struct FrameVertex* frameVertices = (struct FrameVertex*)movieFrame;
	movieFrame += sizeof(struct FrameVertex) * frameInfos->vertexCount;
	struct FrameFace* frameFaces = (struct FrameFace*)movieFrame;
	movieFrame += sizeof(struct FrameFace) * frameInfos->faceCount;
	struct FrameColor* frameColors = (struct FrameColor*)movieFrame;
	movieFrame += sizeof(struct FrameColor) * frameInfos->colorCount;

	everyOtherFrame++;
	if ((everyOtherFrame&3) != 3)
	{
		movieFrame = currentFrame;
	}
	// set palette

	for (int i = 0; i<frameInfos->colorCount; i++, frameColors++)
	{
		palette[frameColors->index + 50] = frameColors->b + (frameColors->g << 8) + (frameColors->r << 16) + 0xFF000000;
	}
	//DrawTriangle(screenpos[i0], screenpos[i1], screenpos[i2], colors[i]);
	struct FrameFace* face = frameFaces;
	for (int i = 0; i < frameInfos->faceCount; i++, face++)
	{
		struct FrameVertex vc = frameVertices[face->a];
		struct FrameVertex vb = frameVertices[face->b];
		struct FrameVertex va = frameVertices[face->c];

		
		DrawTriangleMovie(va.x, va.y, vb.x, vb.y, vc.x, vc.y, face->colorIndex + 50);
	}
	return 1;
}