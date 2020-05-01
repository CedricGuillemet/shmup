
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char GLubyte;
typedef unsigned int GLsizei;
typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned char GLenum;

#define GL_LUMINANCE 0
#define GL_LUMINANCE_ALPHA 1
#define GL_RGB 2
#define GL_RGBA 3

/* OpenGL texture info */
struct gl_texture_t
{
    GLsizei width;
    GLsizei height;

    GLenum format;
    GLint	internalFormat;
    GLuint id;

    GLubyte* texels;
};

#pragma pack(push, 1)
/* TGA header */
struct tga_header_t
{
    GLubyte id_lenght;          /* size of image id */
    GLubyte colormap_type;      /* 1 is has a colormap */
    GLubyte image_type;         /* compression type */

    short	cm_first_entry;       /* colormap origin */
    short	cm_length;            /* colormap length */
    GLubyte cm_size;            /* colormap size */

    short	x_origin;             /* bottom left x coord origin */
    short	y_origin;             /* bottom left y coord origin */

    short	width;                /* picture width (in pixels) */
    short	height;               /* picture height (in pixels) */

    GLubyte pixel_depth;        /* bits per pixel: 8, 16, 24 or 32 */
    GLubyte image_descriptor;   /* 24 bits = 0x00; 32 bits = 0x80 */
};
#pragma pack(pop)

struct gl_texture_t* ReadTGAFile(const char* filename);