struct Fixed angle;
void DrawLevel()
{
    struct Fixed fovy, aspect, zn, zf;
    SetFixed(&fovy, 60622);
    SetFixed(&aspect, (65536 * 4) / 3);
    SetFixed(&zn, 32768);
    SetFixed(&zf, 65536 * 1000);
    struct Matrix_t perspective = PerspectiveFov(fovy, aspect, zn, zf);


    angle = Sub(angle, FromFixed(600));
    struct Fixed circular = RadianToCircular(angle);
    struct Vector3 eye = V3Mul(V3FromFixed(Cosine(circular), Add(Mul(Sine(circular), FromFixed(0x4000)), FromFixed(0x4000)), Sine(circular)), Add(FromInt(4), Cosine(circular)));


    struct Matrix_t view = LookAt(eye, V3FromInt(0, 0, 0), V3FromInt(0, 1, 0));
    struct Matrix_t clipSpaceTo2D = IdentityMatrix();
    clipSpaceTo2D.v[0] = FromInt(160);
    clipSpaceTo2D.v[5] = FromInt(-100); // Y is inverted because of framebuffer. top of array in screen bottom
    clipSpaceTo2D.v[12] = FromInt(160);
    clipSpaceTo2D.v[13] = FromInt(100);
    struct Matrix_t perspectiveScreen = MulMatrix(perspective, clipSpaceTo2D);
    struct Matrix_t vp = MulMatrix(view, perspectiveScreen);

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

    struct Vector2 screenpos[8];
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
        DrawTri(screenpos[i0], screenpos[i1], screenpos[i2], colors[i]);
        DrawTri(screenpos[i0], screenpos[i2], screenpos[i3], colors[i]);
    }

    for (int i = 0; i < 8; i++)
    {
        //screenpos[i] = TransformV3I8(&vp, &v[i * 3]);
        set_pixel(screenpos[i].x.integer, screenpos[i].y.integer, 15);
    }
    set_pixel(horizonPosScreen.x.integer, horizonPosScreen.y.integer, 0);
}