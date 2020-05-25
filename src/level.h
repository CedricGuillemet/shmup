


void ComputeMatrices()
{
    //SetFixed(&fovy, 60622);
    SetFixed(&aspect, (65536 * 320) / 200);
    SetFixed(&zn, 32768);
    SetFixed(&zf, 65536 * 1000);
    struct Matrix_t perspective = PerspectiveFov(fovy, aspect, zn, zf);


    angle = Sub(angle, FromFixed(600));
    struct Fixed circular = RadianToCircular(angle);
    eye = V3Mul(V3FromFixed(Cosine(circular), Add(Mul(Sine(circular), FromFixed(0x4000)), FromFixed(0x4000)), Sine(circular)), Add(FromInt(12), Cosine(circular)));


    view = LookAt(eye, V3FromInt(0, 0, 0), V3FromInt(0, 1, 0));
    clipSpaceTo2D = IdentityMatrix();
    clipSpaceTo2D.v[0] = FromInt(160);
    clipSpaceTo2D.v[5] = FromInt(-100); // Y is inverted because of framebuffer. top of array in screen bottom
    clipSpaceTo2D.v[12] = FromInt(160);
    clipSpaceTo2D.v[13] = FromInt(100);
    perspectiveScreen = MulMatrix(perspective, clipSpaceTo2D);
    vp = MulMatrix(view, perspectiveScreen);

    // game space view
    gameView = LookAt(V3FromInt(16, 10, 20), V3FromInt(16, 10, 0), V3FromInt(0, -1, 0));
    gameVP = MulMatrix(gameView, perspectiveScreen);
}

struct QuadMesh
{
    char* positions;
    unsigned char* quads;
    unsigned char* quadColors;
    int positionCount;
    int quadCount;
};

struct QuadMesh* generateBevel(int sliceCount, struct Vector3* dif, unsigned char* colors)
{
    struct QuadMesh* quadMesh = (struct QuadMesh*)malloc(sizeof(struct QuadMesh));

    quadMesh->positionCount = sliceCount * 4;
    quadMesh->quadCount = (sliceCount - 1) * 4 + 1;
    quadMesh->positions = (char*)malloc(quadMesh->positionCount * 3);
    quadMesh->quads = (unsigned char*)malloc(quadMesh->quadCount);
    quadMesh->quadColors = (unsigned char*)malloc(quadMesh->quadCount);

    // positions
    char* posPtr = quadMesh->positions;
    for (int h = 0; h < sliceCount; h++)
    {
        struct Vector3 pos[4];
        struct Fixed invX = Sub(FromInt(1), dif[h].x);
        struct Fixed invZ = Sub(FromInt(1), dif[h].z);
        pos[0] = dif[h];
        pos[1] = V3FromFixed(invX,     dif[h].y, dif[h].z);
        pos[2] = V3FromFixed(invX, dif[h].y, invZ);
        pos[3] = V3FromFixed(dif[h].x, dif[h].y, invZ);
        
        for (int i = 0; i < 4; i++)
        {
            *posPtr++ = pos[i].x.value / 16384;
            *posPtr++ = pos[i].y.value / 16384;
            *posPtr++ = pos[i].z.value / 16384;
        }
    }

    // quads
    unsigned char* ptr = quadMesh->quads;
    for (int h = 0; h < (sliceCount - 1); h++)
    {
        int baseIndex = h * 4;
        for (int i = 0;i < 4; i++)
        {
            *ptr++ = h + 4 + i;
            *ptr++ = h + 4 + ((i + 1) & 3);
            *ptr++ = h + ((i + 1) & 3);
            *ptr++ = h + i;
        }
    }

    // cap
    *ptr++ = (sliceCount - 1) * 4 + 3;
    *ptr++ = (sliceCount - 1) * 4 + 2;
    *ptr++ = (sliceCount - 1) * 4 + 1;
    *ptr++ = (sliceCount - 1) * 4 + 0;

    // colors
    unsigned char* colPtr = quadMesh->quadColors;
    for (int h = 0; h < (sliceCount - 1); h++)
    {
        for (int i = 0; i < 4; i++)
        {
            *colPtr++ = colors[h];
        }
    }
    // cap
    *colPtr = colors[sliceCount - 2];
    return quadMesh;
}

struct QuadMesh* bevel;
void InitLevels()
{
    struct Vector3 vts[2] = {V3FromInt(0,0,0), V3FromFixed(FromFixed(0x4000),FromInt(1),FromFixed(0x4000)) };
    unsigned char color = 0;
    bevel = generateBevel(2, vts, &color);
}

void DrawQuadMesh(struct Matrix_t* matrix, struct QuadMesh* quadMesh)
{
    struct Vector2 screenpos[64];
    for (int i = 0; i < quadMesh->positionCount; i++)
    {
        screenpos[i] = TransformV3I8(&vp, &quadMesh->positions[i * 3]);
    }

    unsigned char* quadPtr = quadMesh->quads;
    unsigned char* colorPtr = quadMesh->quadColors;
    for (int i = 0; i < quadMesh->quadCount; i++)
    {
        unsigned char i0 = *quadPtr++;
        unsigned char i1 = *quadPtr++;
        unsigned char i2 = *quadPtr++;
        unsigned char i3 = *quadPtr++;
        unsigned char color = *colorPtr++;
        DrawTriangle(screenpos[i0], screenpos[i1], screenpos[i2], color);
        DrawTriangle(screenpos[i0], screenpos[i2], screenpos[i3], color);
    }
}

void DrawLevel()
{
    struct Vector2 screenpos[64];
    struct Vector3 horizonPos = V3Mul(V3Normalize(V3FromFixed(eye.x, FromInt(0), eye.z)), FromInt(-100));
    struct Vector2 horizonPosScreen = TransformV3V3(&vp, horizonPos);
    int groundHeight = horizonPosScreen.y.integer;

    // clear
    uint8_t* backBufferPtr = buffer;
    memset(backBufferPtr, 17, SCREEN_WIDTH * groundHeight);
    backBufferPtr += SCREEN_WIDTH * groundHeight;
    memset(backBufferPtr, 18, SCREEN_WIDTH * (SCREEN_HEIGHT - groundHeight));

    DrawQuadMesh(&vp, bevel);

    /*
    char v[8 * 3] = {
        -1,-1,-1,
        -1,-1, 1,
         1,-1, 1,
         1,-1,-1,

        -1, 1,-1,
        -1, 1, 1,
         1, 1, 1,
         1, 1,-1,
    };

    unsigned char quads[6 * 4] = {
        3,2,1,0,
        0,4,7,3,
        3,7,6,2,
        2,6,5,1,
        1,5,4,0,
        4,5,6,7,
    };
    unsigned char colors[6] = {
        2,4,6,8,10,12
    };

    for (int i = 0; i < 8; i++)
    {
        screenpos[i] = TransformV3I8(&vp, &v[i * 3]);
    }
    
    for (int i = 0; i < 6; i++)
    {
        unsigned char i0 = quads[i * 4 + 0];
        unsigned char i1 = quads[i * 4 + 1];
        unsigned char i2 = quads[i * 4 + 2];
        unsigned char i3 = quads[i * 4 + 3];
        DrawTriangle(screenpos[i0], screenpos[i1], screenpos[i2], colors[i]);
        DrawTriangle(screenpos[i0], screenpos[i2], screenpos[i3], colors[i]);
    }
    */

    DrawQuadMesh(&vp, bevel);
}






/*}
}
/*for (int i = 0; i < 8; i++)
{
    //screenpos[i] = TransformV3I8(&vp, &v[i * 3]);
    setPixel(screenpos[i].x.integer, screenpos[i].y.integer, 15);
}
setPixel(horizonPosScreen.x.integer, horizonPosScreen.y.integer, 0);
*/
/*
for (int i = 0; i < 8; i++)
{
    screenpos[i] = TransformV3I8(&mvp, &v[i * 3]);
}

for (int i = 0; i < 6; i++)
{
    unsigned char i0 = quads[i * 4 + 0];
    unsigned char i1 = quads[i * 4 + 1];
    unsigned char i2 = quads[i * 4 + 2];
    unsigned char i3 = quads[i * 4 + 3];
    DrawTriangle(screenpos[i0], screenpos[i1], screenpos[i2], colors[i]);
    DrawTriangle(screenpos[i0], screenpos[i2], screenpos[i3], colors[i]);
}
*/

/*struct Vector3 corners[4] = { V3FromInt(-16, -10, 0),
V3FromInt(-16,  10, 0),
V3FromInt(16, -10, 0),
V3FromInt(16,  10, 0) };
for (int c = 0; c < 4; c++)
{
    screenpos[0] = TransformV3V3(&gameVP, corners[c]);
    setPixel(screenpos[0].x.integer, screenpos[0].y.integer, 16);
}
*/
/*
for (int y = 0;y<3;y++)
{
    for (int x = 0;x<3;x++)
    {
        struct Matrix_t model = TranslateScale(FromFixed(0x8000 + 0x2000 * y), FromFixed(0x15000 - 0x4000*x), FromFixed(0x8000 + 0x2000 * y),
            FromInt( (x-1) * 4 ), FromFixed(0), FromInt((y - 1) * 4) );


        struct Matrix_t mvp;

        mvp = MulMatrix(model, vp);
        */