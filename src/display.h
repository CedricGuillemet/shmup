
uint8_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

void ClearBuffer()
{
    memset(buffer, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
}

void DrawRectangle(struct Vector2 center, struct Vector2 halfExtend, uint8_t color)
{
    int width = halfExtend.x.parts.integer * 2;
    int height = halfExtend.y.parts.integer * 2;

    int startx = center.x.parts.integer - halfExtend.x.parts.integer;
    int starty = center.y.parts.integer - halfExtend.y.parts.integer;

    int endx = center.x.parts.integer + halfExtend.x.parts.integer;
    int endy = center.y.parts.integer + halfExtend.y.parts.integer;

    for (int y = starty; y< endy; y++)
    {
        if (y < 0 || y >= SCREEN_HEIGHT)
        {
            continue;
        }
        for (int x = startx; x<endx; x++)
        {
            if (x < 0 || x >= SCREEN_WIDTH)
            {
                continue;
            }
            buffer[y * SCREEN_WIDTH + x] = color;
        }
    }
}

void DrawRectangle2(struct Vector2 start, struct Vector2 end, uint8_t color)
{
    int startx = start.x.parts.integer;
    int starty = start.y.parts.integer;

    int endx = end.x.parts.integer;
    int endy = end.y.parts.integer;

    for (int y = starty; y < endy; y++)
    {
        if (y < 0 || y >= SCREEN_HEIGHT)
        {
            continue;
        }
        for (int x = startx; x < endx; x++)
        {
            if (x < 0 || x >= SCREEN_WIDTH)
            {
                continue;
            }
            buffer[y * SCREEN_WIDTH + x] = color;
        }
    }
}

int orient2d(struct Vector2 a, struct Vector2 b, struct Vector2 c)
{
    return (b.x.parts.integer - a.x.parts.integer) * (c.y.parts.integer - a.y.parts.integer) - (b.y.parts.integer - a.y.parts.integer) * (c.x.parts.integer - a.x.parts.integer);
}

int min2(int a, int b)
{
    return (a < b)?a:b;
}

int max2(int a, int b)
{
    return (a > b) ? a : b;
}

int min3(int a, int b, int c)
{
    return min2(a, min2(b, c));
}

int max3(int a, int b, int c)
{
    return max2(a, max2(b, c));
}

void setPixelNoCheck(int x, int y, uint8_t color)
{
    buffer[y * SCREEN_WIDTH + x] = color;
}

void setPixel(int x, int y, uint8_t color)
{
    if (y < 0 || y >= SCREEN_HEIGHT || x < 0 || x >= SCREEN_WIDTH)
    {
        return;
    }
    setPixelNoCheck(x, y, color);
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}



void DrawTriangle(struct Vector2 v0, struct Vector2 v1, struct Vector2 v2, unsigned char color)
{
    // Compute triangle bounding box
    int minX = min3(v0.x.parts.integer, v1.x.parts.integer, v2.x.parts.integer);
    int minY = min3(v0.y.parts.integer, v1.y.parts.integer, v2.y.parts.integer);
    int maxX = max3(v0.x.parts.integer, v1.x.parts.integer, v2.x.parts.integer);
    int maxY = max3(v0.y.parts.integer, v1.y.parts.integer, v2.y.parts.integer);

    // Clip against screen bounds
    minX = max(minX, 0);
    minY = max(minY, 0);
    maxX = min(maxX, 320 - 1);
    maxY = min(maxY, 200 - 1);

    // Rasterize
    int x, y;
    for (y = minY; y <= maxY; y++) {
        for (x = minX; x <= maxX; x++) {
            // Determine barycentric coordinates
            struct Vector2 p;
            p.x = FromInt(x);
            p.y = FromInt(y);

            int w0 = orient2d(v1, v2, p);
            int w1 = orient2d(v2, v0, p);
            int w2 = orient2d(v0, v1, p);

            // If p is on or inside all edges, render pixel.
            if (w0 >= 0 && w1 >= 0 && w2 >= 0)
            {
                setPixelNoCheck(x, y, color);
            }
        }
    }
}


void DrawTriangleMovie(int16_t ax, int16_t ay, int16_t bx, int16_t by, int16_t cx, int16_t cy, uint8_t colorIndex)
{
    struct Vector2 va = V2FromInt(ax, ay);
    struct Vector2 vb = V2FromInt(bx, by);
    struct Vector2 vc = V2FromInt(cx, cy);

    DrawTriangle(va, vb, vc, colorIndex);
}

void DrawCircle(struct Vector2 position, int radiusOut, int radiusIn, unsigned char color)
{
    int startx = max(position.x.parts.integer - radiusOut, 0);
    int endx = min(position.x.parts.integer + radiusOut, SCREEN_WIDTH);

    int starty = max(position.y.parts.integer - radiusOut, 0);
    int endy = min(position.y.parts.integer + radiusOut, SCREEN_HEIGHT);

    int radiusOutSq = radiusOut * radiusOut;
    int radiusInSq = radiusIn * radiusIn;

    for (int y = starty; y < endy; y++)
    {
        for (int x = startx; x < endx; x++)
        {
            int dx = x - position.x.parts.integer;
            int dy = y - position.y.parts.integer;
            int distSq = dx * dx + dy * dy;
            if (distSq < radiusOutSq && distSq >= radiusInSq)
            {
                setPixelNoCheck(x, y, color);
            }
        }
    }
}

void DrawLine(int x0, int y0, int x1, int y1, unsigned char color)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;) {
        setPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void DrawBeam(struct Vector2 pta, struct Vector2 ptb, int side, unsigned char color)
{
    struct Vector2 dir = V2Normalize(V2Sub(ptb, pta));
    struct Vector2 right;
    right.x = Neg(dir.y);
    right.y = dir.x;

    struct Vector2 rl = V2Mul(right, FromInt(side));
    struct Vector2 pts[4] = {V2Add(pta, rl), V2Add(ptb, rl), V2Sub(ptb, rl), V2Sub(pta, rl)};

    DrawTriangle(pts[2], pts[1], pts[0], color);
    DrawTriangle(pts[0], pts[3], pts[2], color);
}

void DrawSprite(struct Vector2 pt, uint8_t* sprite, int width, int height, bool inverse)
{
    int sy = pt.y.parts.integer - height / 2;
    int ey = sy + height;
    int sx = pt.x.parts.integer - width / 2;
    int ex = sx + width;

    int index = 0;
    if (inverse)
    {
        for (int y = ey; y >= sy; y--)
        {
            for (int x = sx; x < ex; x++)
            {
                unsigned char col = sprite[index++];
                if (col)
                {
                    setPixel(x, y, col);
                }
            }
        }
    }
    else
    {
        for (int y = sy ; y < ey; y++)
        {
            for (int x = sx; x < ex; x++)
            {
                unsigned char col = sprite[index++];
                if (col)
                {
                    setPixel(x, y, col);
                }
            }
        }
    }
}

#define MAX_TRAILS 32

void DrawSpeed()
{
    memset(buffer, 0, 320*36);

    for (int x = 0; x < 320; x++)
    {
        for (int y = 0;y<100;y++)
        {
            unsigned char col = (y + (fastrand()&3))>> 4;
            col |= 32;
            setPixelNoCheck(x, y+36, col);
            setPixelNoCheck(x, 200 - y - 1, col);
        }
    }

    static bool setup = false;
    static short val[MAX_TRAILS];
    static int curLines[MAX_TRAILS];
    static int thickness[MAX_TRAILS];
    static int shift[MAX_TRAILS];
    static int factors[MAX_TRAILS];
    static int currentTrailCount = 8;
    static int currentSpeed = 24;
    static int animAv = 0;

    if (!setup)
    {
        setup = true;
        for (int i = 0; i < MAX_TRAILS; i++)
        {
            curLines[i] = fastrand() % 192;
            val[i] = (fastrand() % 1536) - 768;
            thickness[i] = (fastrand()&7) + 1;
            shift[i] = (fastrand()&3) + 2;
            factors[i] = (fastrand() & 7) + 1;
        }
    }
    for (int i = 0;i< currentTrailCount;i++)
    {
        for (int y = 0; y < thickness[i]; y++)
        {
            short offset = fastrand() & 0x7;
            for (int x = 0;x<320;x++)
            {
                unsigned short nval = (val[i] + x) * factors[i] + offset;
                unsigned char col = 0;
                if (nval < 128)
                    col = (nval>>4)&0xF;
                else if (nval < 1024)
                    col = 0xF;
                else if (nval < 1024+128)
                    col = ((1024 + 128 - nval)>>4)&0xF;
                else
                    continue;

                col >>= shift[i];
                col |= 32;

                setPixelNoCheck(x, curLines[i] + y, col);
            }
        }
    }

    for (int i = 0;i< currentTrailCount;i++)
    {
        val[i] += currentSpeed;
        if (val[i] > 1536 )
        {
            curLines[i] = fastrand() % 192;
            val[i] = -(fastrand()&0x1FF) - 40 * 24;
            shift[i] = fastrand() & 3;
        }
    }
    
    animAv ++;
    if (animAv == 2)
    {
        animAv = 0;
        currentSpeed += 4;
        currentTrailCount += 2;
        currentSpeed = (currentSpeed>32)? 32 : currentSpeed;
        currentTrailCount = (currentTrailCount > MAX_TRAILS) ? MAX_TRAILS : currentTrailCount;
    }
}

void DrawMesh(struct Matrix_t* matrix, char* positions, int positionCount, unsigned char* triangles, unsigned char* triangleColors, int triangleCount)
{
    struct Vector2 screenpos[64];
    for (int c = 0; c < positionCount; c++)
    {
        screenpos[c] = TransformV3I8(matrix, &positions[c * 3]);
    }

    for (int i = 0; i < triangleCount; i++)
    {
        unsigned char i0 = triangles[i * 3 + 0];
        unsigned char i1 = triangles[i * 3 + 1];
        unsigned char i2 = triangles[i * 3 + 2];
        DrawTriangle(screenpos[i0], screenpos[i1], screenpos[i2], triangleColors[i]);
    }
}

unsigned char glyph[21 * 5 * 8];
void DrawText(int px, int py, const char* text)
{
    do
    {
        char c = *text++;
        unsigned char* pg = &glyph[glyphTable[c]];
        for (int y = 0; y < 8; y++)
        {
            unsigned char g = *pg++;
            unsigned char mask = 1;
            for (int x = 0; x < 8; x++, mask <<= 1)
            {
                if (!(g & mask))
                {
                    //setPixel(px - y , py + x, 15);
                    setPixel(px + x, py + y, 15);
                }
            }
        }
        //py += 8;
        px += 8;
    } while (*text);
}

void DrawNumber(int px, int py, unsigned int number, unsigned int denum)
{
    while (denum >= 1)
    {
        unsigned int v = number / denum;
        number %= denum;
        denum /= 10;
        unsigned char* pg = &glyph[glyphTable['0' + v]];
        for (int y = 0; y < 8; y++)
        {
            unsigned char g = *pg++;
            unsigned char mask = 1;
            for (int x = 0; x < 8; x++, mask <<= 1)
            {
                if (!(g & mask))
                {
                    setPixel(px + x, py + y, 15);
                }
            }
        }
        px += 8;
    };
}
