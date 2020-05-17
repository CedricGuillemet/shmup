
struct Matrix_t PerspectiveFov(struct Fixed fovy, struct Fixed aspect, struct Fixed zn, struct Fixed zf)
{
    struct Matrix_t res;
    struct Fixed* m = res.v;

    struct Fixed tanHalfFov = Tan(Mul(RadianToCircular(fovy), FromFixed(0x8000)));
    m[0 * 4 + 0] = Div(FromInt(1), Mul(aspect, tanHalfFov));
    SetInt(&m[0 * 4 +1], 0);
    SetInt(&m[0 * 4 +2], 0);
    SetInt(&m[0 * 4 +3], 0);

    SetInt(&m[1 * 4 +0], 0);
    m[1 * 4 + 1] = Div(FromInt(1), tanHalfFov);
    SetInt(&m[1 * 4 +2], 0);
    SetInt(&m[1 * 4 +3], 0);

    SetInt(&m[2 * 4 +0], 0);
    SetInt(&m[2 * 4 +1], 0);
    m[2 * 4 +2] = Div(zf, Sub(zf, zn));
    SetInt(&m[2 * 4 +3], 1);

    SetInt(&m[3 * 4 +0], 0);
    SetInt(&m[3 * 4 +1], 0);
    m[3 * 4 + 2] = Div(Mul(zf, zn), Sub(zn, zf));
    SetInt(&m[3 * 4 +3], 0);

    return res;
}

struct Matrix_t LookAt(struct Vector3 eye, struct Vector3 at, struct Vector3 up)
{
    struct Matrix_t res;
    struct Fixed* m = res.v;

    struct Vector3 X, Y, Z, tmp;

    tmp = V3Sub(at, eye);
    Z = V3Normalize(tmp);
    Y = V3Normalize(up);

    tmp = V3Cross(Y, Z);
    X = V3Normalize(tmp);

    tmp = V3Cross(Z, X);
    Y = V3Normalize(tmp);

    m[0 * 4 + 0] = X.x;
    m[0 * 4 + 1] = Y.x;
    m[0 * 4 + 2] = Z.x;
    SetInt(&m[0 * 4 + 3], 0);

    m[1 * 4 + 0] = X.y;
    m[1 * 4 + 1] = Y.y;
    m[1 * 4 + 2] = Z.y;
    SetInt(&m[1 * 4 + 3], 0);

    m[2 * 4 + 0] = X.z;
    m[2 * 4 + 1] = Y.z;
    m[2 * 4 + 2] = Z.z;
    SetInt(&m[2 * 4 + 3], 0);

    SetFixed(&m[3 * 4 + 0], -V3Dot(X, eye).value);
    SetFixed(&m[3 * 4 + 1], -V3Dot(Y, eye).value);
    SetFixed(&m[3 * 4 + 2], -V3Dot(Z, eye).value);
    SetInt(&m[3 * 4 + 3], 1);

    return res;
}

struct Matrix_t MulMatrix(struct Matrix_t mata, struct Matrix_t matb)
{
    struct Matrix_t res;
    struct Fixed* r = res.v;
    struct Fixed* a = mata.v;
    struct Fixed* b = matb.v;

    r[0] = Add(Add(Mul(a[0], b[0]), Mul(a[1], b[4])), Add(Mul(a[2], b[8]), Mul(a[3], b[12])));
    r[1] = Add(Add(Mul(a[0], b[1]), Mul(a[1], b[5])), Add(Mul(a[2], b[9]), Mul(a[3], b[13])));
    r[2] = Add(Add(Mul(a[0], b[2]), Mul(a[1], b[6])), Add(Mul(a[2], b[10]), Mul(a[3], b[14])));
    r[3] = Add(Add(Mul(a[0], b[3]), Mul(a[1], b[7])), Add(Mul(a[2], b[11]), Mul(a[3], b[15])));

    r[4] = Add(Add(Mul(a[4], b[0]), Mul(a[5], b[4])), Add(Mul(a[6], b[8]), Mul(a[7], b[12])));
    r[5] = Add(Add(Mul(a[4], b[1]), Mul(a[5], b[5])), Add(Mul(a[6], b[9]), Mul(a[7], b[13])));
    r[6] = Add(Add(Mul(a[4], b[2]), Mul(a[5], b[6])), Add(Mul(a[6], b[10]), Mul(a[7], b[14])));
    r[7] = Add(Add(Mul(a[4], b[3]), Mul(a[5], b[7])), Add(Mul(a[6], b[11]), Mul(a[7], b[15])));

    r[8] = Add(Add(Mul(a[8], b[0]), Mul(a[9], b[4])), Add(Mul(a[10], b[8]), Mul(a[11], b[12])));
    r[9] = Add(Add(Mul(a[8], b[1]), Mul(a[9], b[5])), Add(Mul(a[10], b[9]), Mul(a[11], b[13])));
    r[10] = Add(Add(Mul(a[8], b[2]), Mul(a[9], b[6])), Add(Mul(a[10], b[10]), Mul(a[11], b[14])));
    r[11] = Add(Add(Mul(a[8], b[3]), Mul(a[9], b[7])), Add(Mul(a[10], b[11]), Mul(a[11], b[15])));

    r[12] = Add(Add(Mul(a[12], b[0]), Mul(a[13], b[4])), Add(Mul(a[14], b[8]), Mul(a[15], b[12])));
    r[13] = Add(Add(Mul(a[12], b[1]), Mul(a[13], b[5])), Add(Mul(a[14], b[9]), Mul(a[15], b[13])));
    r[14] = Add(Add(Mul(a[12], b[2]), Mul(a[13], b[6])), Add(Mul(a[14], b[10]), Mul(a[15], b[14])));
    r[15] = Add(Add(Mul(a[12], b[3]), Mul(a[13], b[7])), Add(Mul(a[14], b[11]), Mul(a[15], b[15])));
    return res;
}

struct Vector2 TransformV3I8(struct Matrix_t* matrix, char *p)
{
    struct Vector2 res;

    struct Vector3 pos = V3FromChar(p[0], p[1], p[2]);

    struct Fixed outx = Add(Add(Mul(pos.x, matrix->v[0 * 4 + 0]), Mul(pos.y, matrix->v[1 * 4 + 0])), Add(Mul(pos.z, matrix->v[2 * 4 + 0]), matrix->v[3 * 4 + 0]));
    struct Fixed outy = Add(Add(Mul(pos.x, matrix->v[0 * 4 + 1]), Mul(pos.y, matrix->v[1 * 4 + 1])), Add(Mul(pos.z, matrix->v[2 * 4 + 1]), matrix->v[3 * 4 + 1]));
    //struct Fixed outz = Add(Add(Mul(pos.x, matrix->v[0 * 4 + 2]), Mul(pos.y, matrix->v[1 * 4 + 2])), Add(Mul(pos.z, matrix->v[2 * 4 + 2]), matrix->v[3 * 4 + 2]));
    struct Fixed outw = Add(Add(Mul(pos.x, matrix->v[0 * 4 + 3]), Mul(pos.y, matrix->v[1 * 4 + 3])), Add(Mul(pos.z, matrix->v[2 * 4 + 3]), matrix->v[3 * 4 + 3]));
    /*if (outz.value < 0 )
    {
        res.x = FromInt(0);
        res.y = FromInt(0);

        return res;
    }*/
    res.x = Div(outx, outw);
    res.y = Div(outy, outw);

    return res;
}


struct Vector2 TransformV3V3(struct Matrix_t* matrix, struct Vector3 pos)
{
    struct Vector2 res;

    struct Fixed outx = Add(Add(Mul(pos.x, matrix->v[0 * 4 + 0]), Mul(pos.y, matrix->v[1 * 4 + 0])), Add(Mul(pos.z, matrix->v[2 * 4 + 0]), matrix->v[3 * 4 + 0]));
    struct Fixed outy = Add(Add(Mul(pos.x, matrix->v[0 * 4 + 1]), Mul(pos.y, matrix->v[1 * 4 + 1])), Add(Mul(pos.z, matrix->v[2 * 4 + 1]), matrix->v[3 * 4 + 1]));
    //struct Fixed outz = Add(Add(Mul(pos.x, matrix->v[0 * 4 + 2]), Mul(pos.y, matrix->v[1 * 4 + 2])), Add(Mul(pos.z, matrix->v[2 * 4 + 2]), matrix->v[3 * 4 + 2]));
    struct Fixed outw = Add(Add(Mul(pos.x, matrix->v[0 * 4 + 3]), Mul(pos.y, matrix->v[1 * 4 + 3])), Add(Mul(pos.z, matrix->v[2 * 4 + 3]), matrix->v[3 * 4 + 3]));

    res.x = Div(outx, outw);
    res.y = Div(outy, outw);

    return res;
}
