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