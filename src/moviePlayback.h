#pragma once

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

#define MOVIE_SLOT_COUNT 32

#define MOVIE_COLOR_OFFSET_BACKGROUND 200
#define MOVIE_COLOR_OFFSET_FOREGROUND 50
#define MOVIE_COLOR_COUNT_BACKGROUND 50
#define MOVIE_COLOR_COUNT_FOREGROUND 150


extern int ReadMovie(const char* szPath);
extern int RenderMovieFrame();

// slow/debug
extern int GetMovieFrameCount();
extern void RenderMovieFrame(int frameIndex);
