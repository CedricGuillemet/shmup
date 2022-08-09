#pragma once

struct FrameInfos
{
	uint16_t faceCount;
	uint8_t vertexCount;
	uint8_t colorCount;
};
struct FrameVertex
{
	int8_t x, y;
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

extern int ReadMovie(const char* szPath);
extern int RenderMovieFrame();