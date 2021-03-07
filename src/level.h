


void ComputeMatrices()
{
    //SetFixed(&fovy, 60622);
    SetFixed(&aspect, (65536 * 320) / 200);
    SetFixed(&zn, 32768);
    SetFixed(&zf, 65536 * 1000);
    struct Matrix_t perspective = PerspectiveFov(fovy, aspect, zn, zf);


    angle = Sub(angle, FromFixed(600));
    struct Fixed circular = RadianToCircular(angle);
    eye = V3Mul(V3FromFixed(Cosine(circular), Add(Mul(Sine(circular), FromFixed(0x4000)), FromFixed(0x10000)), Sine(circular)), Add(FromInt(5), Cosine(circular)));


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
    struct Vector3* positions;
    unsigned char* quads;
    unsigned char* quadColors;
    int positionCount;
    int quadCount;
};

struct Vector3 GetNormal(struct QuadMesh* quadMesh, int quadIndex)
{
    unsigned char idx0 = quadMesh->quads[quadIndex * 4];
    unsigned char idx1 = quadMesh->quads[quadIndex * 4 + 1];
    unsigned char idx2 = quadMesh->quads[quadIndex * 4 + 3];
    /*char* p0 = &quadMesh->positions[idx0 * 3];
    char* p1 = &quadMesh->positions[idx1 * 3];
    char* p2 = &quadMesh->positions[idx2 * 3];
    struct Vector3 v0 = V3FromChar(p0[0], p0[1], p0[2]);
    struct Vector3 v1 = V3FromChar(p1[0], p1[1], p1[2]);
    struct Vector3 v2 = V3FromChar(p2[0], p2[1], p2[2]);
    */
    struct Vector3 v0 = quadMesh->positions[idx0];
    struct Vector3 v1 = quadMesh->positions[idx1];
    struct Vector3 v2 = quadMesh->positions[idx2];

    struct Vector3 f0 = V3Sub(v1, v0);
    struct Vector3 f1 = V3Sub(v2, v0);

    struct Vector3 n = V3Normalize(V3Cross(f0, f1));
    return n;
}

void ApplyLighting(struct QuadMesh* quadMesh, struct Vector3 lightDir)
{
    for (int q = 0; q < quadMesh->quadCount; q++)
    {
        unsigned char c = quadMesh->quadColors[q];
        if (c == 0 || c >= 128)
        {
            struct Vector3 n = GetNormal(quadMesh, q);
            //struct Fixed dt = Max(V3Dot(n, lightDir), FromFixed(0x1000));
            //unsigned char col = Mul(dt, FromInt(16)).integer & 15;
            struct Fixed dt = Add(Mul(V3Dot(n, lightDir), FromInt(8)), FromInt(8));
            unsigned char col = dt.integer & 15;
            quadMesh->quadColors[q] += col;
        }
    }
}

struct QuadMesh* AllocateQuadMesh(int positionCount, int quadCount)
{
    struct QuadMesh* quadMesh = (struct QuadMesh*)malloc(sizeof(struct QuadMesh));

    quadMesh->positionCount = positionCount;
    quadMesh->quadCount = quadCount;
    quadMesh->positions = (struct Vector3*)malloc(quadMesh->positionCount * sizeof(struct Vector3));
    quadMesh->quads = (unsigned char*)malloc(quadMesh->quadCount * 4);
    quadMesh->quadColors = (unsigned char*)malloc(quadMesh->quadCount);

    return quadMesh;
}

void ReallocateMesh(struct QuadMesh* mesh, int positionCount, int quadCount)
{
    mesh->positionCount = positionCount;
    mesh->quadCount = quadCount;
    mesh->positions = (struct Vector3*)realloc(mesh->positions, mesh->positionCount * sizeof(struct Vector3));
    mesh->quads = (unsigned char*)realloc(mesh->quads, mesh->quadCount * 4);
    mesh->quadColors = (unsigned char*)realloc(mesh->quadColors, mesh->quadCount);
}

struct QuadMesh* GenerateBevel(int sliceCount, struct Vector3* dif, unsigned char* colors)
{
    struct QuadMesh* quadMesh = AllocateQuadMesh(sliceCount * 4, (sliceCount - 1) * 4 + 1);

    // positions
    struct Vector3* posPtr = quadMesh->positions;
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
            *posPtr++ = pos[i];
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
    *colPtr = colors[sliceCount - 1];
    return quadMesh;
}


struct QuadMesh* GenerateRibbon(int quadCount, struct Vector3* positions, unsigned char* colors, struct Vector3 offset)
{
    struct QuadMesh* quadMesh = AllocateQuadMesh(quadCount * 2 + 2, quadCount);

    // positions
    struct Vector3* posPtr = quadMesh->positions;
    for (int h = 0; h < quadMesh->positionCount/2; h++)
    {
        struct Vector3 pos[2];
        pos[0] = positions[h];
        pos[1] = V3Add(positions[h], offset);//V3FromInt(0, 0, 1));

        for (int i = 0; i < 2; i++)
        {
            *posPtr++ = pos[i];
        }
    }

    // quads
    unsigned char* ptr = quadMesh->quads;
    for (int h = 0; h < quadCount; h++)
    {
        int baseIndex = h * 2;
        *ptr++ = baseIndex + 1;
        *ptr++ = baseIndex + 3;
        *ptr++ = baseIndex + 2;
        *ptr++ = baseIndex;
    }

    // colors
    unsigned char* colPtr = quadMesh->quadColors;
    for (int h = 0; h < quadCount; h++)
    {
        *colPtr++ = colors[h];
    }
    return quadMesh;
}

struct QuadMesh* Duplicate(struct QuadMesh* mesh, int count, struct Vector3 offset)
{
    struct QuadMesh* quadMesh = AllocateQuadMesh(mesh->positionCount * count, mesh->quadCount * count);

    struct Vector3* posDestPtr = quadMesh->positions;
    unsigned char* colDestPtr = quadMesh->quadColors;
    unsigned char* ptrDest = quadMesh->quads;

    for (int clone = 0; clone < count; clone++)
    {
        struct Vector3* posPtr = mesh->positions;
        for (int h = 0; h < mesh->positionCount; h++)
        {
            struct Vector3 source = *posPtr++;
            source = V3Add(source, V3Mul(offset, FromInt(clone)));
            *posDestPtr++ = source;
        }
    }

    for (int h = 0; h < mesh->quadCount; h++)
    {
        for (int clone = 0; clone < count; clone++)
        {
            for (int i = 0; i < 4; i++)
            {
                *ptrDest++ = mesh->quads[h * 4 + i] + mesh->positionCount * clone;
            }
            *colDestPtr++ = mesh->quadColors[h];
        }
    }
    
    return quadMesh;
}

void RemoveQuad(struct QuadMesh* mesh, int quadIndex)
{
    mesh->quadCount --;
    memcpy(&mesh->quads[quadIndex * 4], &mesh->quads[mesh->quadCount * 4], sizeof(unsigned char) * 4);
    mesh->quadColors[quadIndex] = mesh->quadColors[mesh->quadCount];

}

void BendMesh(struct QuadMesh* mesh, struct Fixed angle)
{
    //struct Vector3 c = V3FromInt(0,0,0);

    struct Vector3* p = mesh->positions;
    struct Fixed max = p[0].z;
    for (int i = 1; i < mesh->positionCount; i++)
    {
        if (max.value < p[i].z.value)
        {
            max = p[i].z;
        }
    }

    struct Fixed cx = Div(max, angle);

    for (int i = 0; i < mesh->positionCount; i++)
    {
        struct Fixed a = Div(Mul(p[i].z, angle), max);

        struct Vector3 v = p[i];

        v.z.value = 0;
        v.x = Add(v.x, cx);
        v = V3Rotate(v, a);
        v.x = Sub(v.x, cx);

        p[i] = v;
    }
}

void DeformedMesh(struct QuadMesh* mesh, struct QuadMesh* deformMesh, int deformQuadIndex)
{
    //struct QuadMesh* quadMesh = AllocateQuadMesh(mesh->positionCount, mesh->quadCount);
    unsigned char* index = &deformMesh->quads[deformQuadIndex * 4];
    struct Vector3 v0 = deformMesh->positions[index[3]];
    struct Vector3 v1 = deformMesh->positions[index[2]];
    struct Vector3 v2 = deformMesh->positions[index[1]];
    struct Vector3 v3 = deformMesh->positions[index[0]];
    struct Vector3 n = GetNormal(deformMesh, deformQuadIndex);
    for (int i = 0; i < mesh->positionCount; i++)
    {
        struct Vector3 p = mesh->positions[i];
        struct Vector3 d0 = V3Lerp(v0, v1, p.x);
        struct Vector3 d1 = V3Lerp(v3, v2, p.x);
        struct Vector3 d = V3Lerp(d0, d1, p.z);
        d = V3Add(d, V3Mul(n, p.y));
        mesh->positions[i] = d;
    }
}

void AppendMesh(struct QuadMesh* mesh, struct QuadMesh* source)
{
    int basePosition = mesh->positionCount;
    int baseQuad = mesh->quadCount;

    ReallocateMesh(mesh, mesh->positionCount + source->positionCount, mesh->quadCount + source->quadCount);
    for (int i = 0; i < source->positionCount; i++)
    {
        mesh->positions[basePosition + i] = source->positions[i];
    }
    for (int i = 0; i < source->quadCount * 4; i++)
    {
        mesh->quads[baseQuad * 4 + i] = source->quads[i] + basePosition;
    }
    for (int i = 0; i < source->quadCount; i++)
    {
        mesh->quadColors[baseQuad + i] = source->quadColors[i];
    }
}

struct QuadMesh* bevel;
struct QuadMesh* ribbon;

void InitLevels()
{
    BuildGradient(V3FromInt(255,80,20), 128);
    BuildGradient(V3FromInt(124, 73, 29), 128 + 16);
    BuildGradient(V3FromInt(90, 90, 100), 128 + 32);

    struct Vector3 bvts[2] = {V3FromInt(0,0,0), V3FromFixed(FromFixed(0x4000),FromInt(1),FromFixed(0x4000)) };
    unsigned char bcolor[] = {128, 19};
    bevel = GenerateBevel(2, bvts, bcolor);
    //ApplyLighting(bevel, V3Normalize(V3FromInt(1,3,2)));
    /*
    struct Vector3 vts[7] = { V3FromInt(4,-6,0), V3FromInt(6,-1,0), V3FromInt(8,0,0), V3FromInt(10,0,0), V3FromInt(10,1,0), V3FromInt(20,2,0), V3FromInt(30,2,0) };
    unsigned char color[6] = {144,144,144,160,160, 160};
    struct QuadMesh* ribbons = GenerateRibbon(5, vts, color, V3FromInt(0, 0, 1));
    ribbon = Duplicate(ribbons, 16, V3FromInt(0,0,1));

    
    

    BendMesh(ribbon, Neg(FromFixed(411774)));

    struct QuadMesh* pico[8];
    for (int i = 0; i < 8; i++)
    {
        pico[i] = Duplicate(bevel, 1, V3FromInt(0, 0, 0));
        DeformedMesh(pico[i], ribbon, 16 + i * 2);
    }

    for (int i = 0; i < 8; i++)
    {
        RemoveQuad(ribbon, 16 + i * 2);
    }

    for (int i = 0; i < 8; i++)
    {
        AppendMesh(ribbon, pico[i]);
    }
    ApplyLighting(ribbon, V3Normalize(V3FromInt(1,3,2)));

    */
    
    struct Vector3 vts[9] = { V3FromInt(0,0,0), V3FromInt(0,0,1),V3FromInt(0,0,2),V3FromInt(0,0,3),V3FromInt(0,0,4),V3FromInt(0,0,5),V3FromInt(0,0,6),V3FromInt(0,0,7),V3FromInt(0,0,8) };
    unsigned char color[8] = { 144,144,144,144, 144,144,144,144 };
    /*struct QuadMesh* */ribbon = GenerateRibbon(8, vts, color, V3FromInt(0, 1, 0));

    BendMesh(ribbon, Neg(FromFixed(411774)));



    struct QuadMesh* pico[8];
    for (int i = 0; i < 4; i++)
    {
        pico[i] = Duplicate(bevel, 1, V3FromInt(0, 0, 0));
        DeformedMesh(pico[i], ribbon, i * 2);
    }

    for (int i = 0; i < 4; i++)
    {
        RemoveQuad(ribbon, i * 2);
    }

    for (int i = 0; i < 4; i++)
    {
        AppendMesh(ribbon, pico[i]);
    }


    ApplyLighting(ribbon, V3Normalize(V3FromInt(1, 3, 2)));

    //ApplyLighting(pico, V3Normalize(V3FromInt(1, 3, 2)));
}

void DrawQuadMesh(struct Matrix_t* matrix, struct QuadMesh* quadMesh)
{
    struct Vector2 screenpos[1024];
    for (int i = 0; i < quadMesh->positionCount; i++)
    {
        screenpos[i] = TransformV3V3(&vp, quadMesh->positions[i]);
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
    //struct Vector2 screenpos[64];
    struct Vector3 horizonPos = V3Mul(V3Normalize(V3FromFixed(eye.x, FromInt(0), eye.z)), FromInt(-100));
    struct Vector2 horizonPosScreen = TransformV3V3(&vp, horizonPos);
    int groundHeight = horizonPosScreen.y.integer;

    // clear
    
    uint8_t* backBufferPtr = buffer;
    /*
    memset(backBufferPtr, 17, SCREEN_WIDTH * groundHeight);
    backBufferPtr += SCREEN_WIDTH * groundHeight;
    memset(backBufferPtr, 18, SCREEN_WIDTH * (SCREEN_HEIGHT - groundHeight));
    */
    memset(backBufferPtr, 0, SCREEN_WIDTH * 200);

    DrawQuadMesh(&vp, ribbon);
    //DrawQuadMesh(&vp, pico);

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