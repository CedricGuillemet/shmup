uint32_t palette[256] = { 0xFF000000, 0xFFFFFFFF };
uint8_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

void DrawRectangle(struct Vector2 center, struct Vector2 halfExtend, uint8_t color)
{
    int width = halfExtend.x.integer * 2;
    int height = halfExtend.y.integer * 2;

    int startx = center.x.integer - halfExtend.x.integer;
    int starty = center.y.integer - halfExtend.y.integer;

    int endx = center.x.integer + halfExtend.x.integer;
    int endy = center.y.integer + halfExtend.y.integer;

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

int orient2d(struct Vector2 a, struct Vector2 b, struct Vector2 c)
{
    return (b.x.integer - a.x.integer) * (c.y.integer - a.y.integer) - (b.y.integer - a.y.integer) * (c.x.integer - a.x.integer);
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

void DrawTriangle(struct Vector2 v0, struct Vector2 v1, struct Vector2 v2, unsigned char color)
{
    // Compute triangle bounding box
    int minX = min3(v0.x.integer, v1.x.integer, v2.x.integer);
    int minY = min3(v0.y.integer, v1.y.integer, v2.y.integer);
    int maxX = max3(v0.x.integer, v1.x.integer, v2.x.integer);
    int maxY = max3(v0.y.integer, v1.y.integer, v2.y.integer);

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

void DrawCircle(struct Vector2 position, int radiusOut, int radiusIn, unsigned char color)
{
    int startx = max(position.x.integer - radiusOut, 0);
    int endx = min(position.x.integer + radiusOut, SCREEN_WIDTH);

    int starty = max(position.y.integer - radiusOut, 0);
    int endy = min(position.y.integer + radiusOut, SCREEN_HEIGHT);

    int radiusOutSq = radiusOut * radiusOut;
    int radiusInSq = radiusIn * radiusIn;

    for (int y = starty; y < endy; y++)
    {
        for (int x = startx; x < endx; x++)
        {
            int dx = x - position.x.integer;
            int dy = y - position.y.integer;
            int distSq = dx * dx + dy * dy;
            if (distSq < radiusOutSq && distSq > radiusInSq)
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
    int sy = pt.y.integer - height / 2;
    int ey = sy + height;
    int sx = pt.x.integer - width / 2;
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