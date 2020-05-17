struct Fixed angle;

struct Matrix_t view;
struct Matrix_t clipSpaceTo2D;
struct Matrix_t perspectiveScreen;
struct Matrix_t vp;
struct Vector2 screenpos[20];
struct Vector3 eye;
struct Fixed fovy = {60622}, aspect, zn, zf;

void ComputeMatrices()
{
    //SetFixed(&fovy, 60622);
    SetFixed(&aspect, (65536 * 320) / 200);
    SetFixed(&zn, 32768);
    SetFixed(&zf, 65536 * 1000);
    struct Matrix_t perspective = PerspectiveFov(fovy, aspect, zn, zf);


    angle = Sub(angle, FromFixed(600));
    struct Fixed circular = RadianToCircular(angle);
    eye = V3Mul(V3FromFixed(Cosine(circular), Add(Mul(Sine(circular), FromFixed(0x4000)), FromFixed(0x4000)), Sine(circular)), Add(FromInt(4), Cosine(circular)));


    view = LookAt(eye, V3FromInt(0, 0, 0), V3FromInt(0, 1, 0));
    clipSpaceTo2D = IdentityMatrix();
    clipSpaceTo2D.v[0] = FromInt(160);
    clipSpaceTo2D.v[5] = FromInt(-100); // Y is inverted because of framebuffer. top of array in screen bottom
    clipSpaceTo2D.v[12] = FromInt(160);
    clipSpaceTo2D.v[13] = FromInt(100);
    perspectiveScreen = MulMatrix(perspective, clipSpaceTo2D);
    vp = MulMatrix(view, perspectiveScreen);
}

void DrawLevel()
{
    struct Vector3 horizonPos = V3Mul(V3Normalize(V3FromFixed(eye.x, FromInt(0), eye.z)), FromInt(-100));
    struct Vector2 horizonPosScreen = TransformV3V3(&vp, horizonPos);
    int groundHeight = horizonPosScreen.y.integer;

    // clear
    uint8_t* backBufferPtr = buffer;
    memset(backBufferPtr, 17, SCREEN_WIDTH * groundHeight);
    backBufferPtr += SCREEN_WIDTH * groundHeight;
    memset(backBufferPtr, 18, SCREEN_WIDTH * (SCREEN_HEIGHT - groundHeight));

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
}

void DrawShip3D(int flameRadius)
{
    // game space view
    struct Matrix_t gameView = LookAt(V3FromInt(16, 10, 20), V3FromInt(16, 10, 0), V3FromInt(0, -1, 0));
    struct Matrix_t gameVP = MulMatrix(gameView, perspectiveScreen);
    struct Vector3 corners[4] = { V3FromInt(-16, -10, 0),
    V3FromInt(-16,  10, 0),
    V3FromInt(16, -10, 0),
    V3FromInt(16,  10, 0) };
    for (int c = 0; c < 4; c++)
    {
        screenpos[0] = TransformV3V3(&gameVP, corners[c]);
        setPixel(screenpos[0].x.integer, screenpos[0].y.integer, 16);
    }





    struct Matrix_t model = TranslateScale(FromFixed(0x19990), FromFixed(0x19990), FromFixed(0x19990), // div10 = 0x2000
        FromInt(0), FromFixed(0), FromInt(0));


    struct Matrix_t mvp;

    mvp = MulMatrix(model, gameVP);
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
    struct Matrix_t modelScale = TranslateScale(FromFixed(0x300), FromFixed(0x300), FromFixed(0x300), // div10 = 0x2000
        FromFixed(Ship.position.x.value / 10), FromFixed(Ship.position.y.value / 10), FromInt(0));


    struct Matrix_t mvps, modelScaleRot, rotx;

    rotx = RotateX(RadianToCircular(Add(Mul(Add(FromInt(Ship.switchTransition), Mul(FromInt(Ship.upDownMomentum), FromFixed(0x2000))), FromFixed(0x2836)), Ship.isWhite ? FromInt(0) : FromFixed(0x3243F))));

    modelScaleRot = MulMatrix(rotx, modelScale);
    mvps = MulMatrix(modelScaleRot, gameVP);

    if (!(Ship.spawningTransition & 1) && !Ship.dieTransition)
    {

        for (int c = 0; c < 13; c++)
        {
            screenpos[c] = TransformV3I8(&mvps, &shipPositions[c * 3]);
            //setPixel(screenpos[c].x.integer, screenpos[c].y.integer, 16);
        }

        for (int i = 0; i < 14; i++)
        {
            unsigned char i0 = shipTris[i * 3 + 0];
            unsigned char i1 = shipTris[i * 3 + 1];
            unsigned char i2 = shipTris[i * 3 + 2];
            DrawTriangle(screenpos[i0], screenpos[i1], screenpos[i2], shipTrisColor[i]);
        }
    }

    if (flameRadius > 0)
    {
        char circles[9] = { 0,0,0,
                            -127,0,0,
                            -127,40,0 };
        screenpos[0] = TransformV3I8(&mvps, &circles[0]);
        screenpos[1] = TransformV3I8(&mvps, &circles[3]);
        screenpos[2] = TransformV3I8(&mvps, &circles[6]);

        screenpos[1].x = Sub(screenpos[1].x, FromInt(10 + flameRadius));
        DrawCircle(screenpos[1], flameRadius * 2, 0, 15);
        DrawRectangle2(V2FromInt(0, screenpos[1].y.integer - flameRadius), V2FromInt(screenpos[1].x.integer, screenpos[1].y.integer + flameRadius), 15);
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
}