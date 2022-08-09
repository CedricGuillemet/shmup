
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

