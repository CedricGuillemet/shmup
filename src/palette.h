int paletteFade = 0;

uint32_t paletteSource[256] = { 0xFF000000, 0xFFFFFFFF };
uint32_t palette[256] = { 0xFF000000, 0xFFFFFFFF };

void FadePalette(int delta)
{
    unsigned char* ptrSource = (unsigned char*)&paletteSource[0];
    unsigned char* ptrDest = (unsigned char*)&palette[0];
    for (int i = 0; i < 1024; i++)
    {
        int v = ptrSource[i] + delta;
        if (v > 255)
            v = 255;
        if (v < 0)
            v = 0;
        ptrDest[i] = v;
    }
}