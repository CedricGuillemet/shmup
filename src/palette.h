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

void BuildGradient(struct Vector3 diffuse, unsigned char startColor)
{
    for (int i = 0; i < 16; i++)
    {
        struct Vector3 col = V3Lerp(V3FromInt(0,0,0), diffuse, FromFixed(0x1000 * i));
        paletteSource[startColor + i] = V3ToUint32(col);
    }
}