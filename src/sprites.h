
void EndianSwap(uint32_t *buffer, int lengthInBytes)
{
    for (int i = 0; i < lengthInBytes/4;i++)
    {
        uint32_t v = buffer[i];
        //uint32_t v2 = ((v&0xFF)<<0);// + ((v & 0xFF00)<<8) + ((v & 0xFF0000) >> 8) + ((v & 0xFF000000) >> 16);
        
        buffer[i] = (v&0xFF000000) + ((v&0xFF)<<16) + (v & 0xFF00) + ((v & 0xFF0000) >> 16);
    }
}

//struct Input_t Input;
uint8_t* remappedShootWhite;
uint8_t* remappedShootBlack;

uint8_t* RemapBitmap(struct gl_texture_t* texture)
{
    int texSize = texture->width * texture->height;
    EndianSwap((uint32_t*)texture->texels, texSize * 4);

    unsigned char *bitmap = (unsigned char*)malloc(texSize);
    memset(bitmap, 0, texSize);

    int index = 0;
    for (int y = texture->height - 1; y >= 0; y--)
    {
        for (unsigned int x = 0; x < texture->width; x++)
        {
            uint32_t color = ((uint32_t*)texture->texels)[y * texture->width + x];
            for (int j = 1; j < 256; j++)
            {
                if ( palette[j] == color)
                {
                    bitmap[index] = j;
                    break;
                }
            }
            index++;
        }
    }

    return bitmap;
}

void ConvertFontTextureToFontGlyphs(uint32_t* texture)
{
    memset(glyph, 0, sizeof(glyph));
    int out = 0;
    for (int h = 5; h >= 0; h--)
    {
        int offsetH = (h * 8 -1) * 21 * 8;
        for (int g = 0; g < 21; g++)
        {
            int offset = offsetH + g * 8;
            for (int y = 0;y<8;y++)
            {
                for (int x = 0;x<8;x++)
                {
                    int index = (offset - y * 21 * 8 + x);
                    if (texture[index]&0xFF)
                    {
                        glyph[out] |= 1 << x;
                    }
                }
                out++;
            }
        }
    }
}

void RotateSprite(uint8_t *source, int width, int height, uint8_t* destination, struct Fixed angle)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0;x < width; x++)
        {
            struct Vector2 coord = V2FromInt(x - width / 2, y - height / 2);
            struct Vector2 rot = V2Rotate(coord, angle);
            int py = rot.y.parts.integer + height / 2;
            int px = rot.x.parts.integer + width / 2;
            if (py < 0 || py >= height || px < 0 || px >= width)
            {
                *destination++ = 0;
            }
            else
            {
                *destination++ = source[py * width + px];
            }
        }
    }
}

void SpritesInit()
{
    
    struct gl_texture_t* paletteFile = ReadTGAFile("palette.tga");
    memcpy(palette, paletteFile->texels, 256 * 4);
    EndianSwap(palette, 256*4);
    memcpy(paletteSource, palette, 256*4);

    struct gl_texture_t* fontFile = ReadTGAFile("font.tga");
    ConvertFontTextureToFontGlyphs((uint32_t*)fontFile->texels);

    struct gl_texture_t* shootFileWhite = ReadTGAFile("shootWhite.tga");
    struct gl_texture_t* shootFileBlack = ReadTGAFile("shootBlack.tga");
    
    
    remappedShootWhite = RemapBitmap(shootFileWhite);
    remappedShootBlack = RemapBitmap(shootFileBlack);

    for (int j = 0; j < 8; j++)
    {
        static const char *tearShootFileNames[8] = {"bulletTear0Black.tga","bulletTear1Black.tga", "bulletTear2Black.tga", "bulletTear3Black.tga",
            "bulletTear0White.tga","bulletTear1White.tga", "bulletTear2White.tga", "bulletTear3White.tga" };
        struct gl_texture_t* shootTear = ReadTGAFile(tearShootFileNames[j]);
        uint8_t* remappedShootTear = RemapBitmap(shootTear);

        for (int i = 0; i < 16; i++)
        {
            struct Fixed angleSpr = RadianToCircular(Mul(FromInt(i), FromFixed(80)));
            int index = (j & 3) * 16 * 16 * 16 + i * 16 * 16;
            RotateSprite(remappedShootTear, 16, 16, &((j < 3) ? tearShotWhite : tearShotBlack)[index], angleSpr);
        }
    }

    for (int j = 0; j < 4; j++)
    {
        static const char* shootFileNames[4][4] = {
            { "bulletBalls0Black.tga","bulletBalls1Black.tga", "bulletBalls2Black.tga", "bulletBalls3Black.tga"},
            { "bulletBalls0White.tga", "bulletBalls1White.tga", "bulletBalls2White.tga", "bulletBalls3White.tga" },
            { "bulletRotor0Black.tga","bulletRotor1Black.tga", "bulletRotor2Black.tga", "bulletRotor3Black.tga"},
            { "bulletRotor0White.tga", "bulletRotor1White.tga", "bulletRotor2White.tga", "bulletRotor3White.tga" } };
        uint8_t* dests[4] = { ballShotBlack, ballShotWhite, rotorShotBlack, rotorShotWhite };
        for (int i = 0; i < 4; i++)
        {
            struct gl_texture_t* shoot = ReadTGAFile(shootFileNames[j][i]);
            uint8_t* remappedShoot = RemapBitmap(shoot);
            memcpy(&dests[j][i*16*16], remappedShoot, 16 * 16);
        }
    }
}
