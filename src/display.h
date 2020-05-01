uint32_t palette[256] = { 0xFF000000, 0xFFFFFFFF };
uint8_t buffer[SCREEN_WIDTH * SCREEN_HEIGHT];

void Rectangle(struct Vector2 center, struct Vector2 halfExtend, uint8_t color)
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

void set_pixel(int x, int y, uint8_t pixel)
{
    if (y < 0 || y >= SCREEN_HEIGHT || x < 0 || x >= SCREEN_WIDTH)
    {
        return;
    }
    buffer[y * SCREEN_WIDTH + x] = pixel;
}

void DrawTri(struct Vector2 v0, struct Vector2 v1, struct Vector2 v2, unsigned char color)
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
                set_pixel(x, y, color);
            }
        }
    }
}