

// circular value : [0..1024] corresponds to [0..pi]
struct Fixed Cosine(struct Fixed circular)
{
    int index = circular.parts.integer & 2047; // /32 = 65536 / 2048
    return cosines[index];
}

struct Fixed Sine(struct Fixed circular)
{
    int index = (circular.parts.integer + 512) & 2047;
    return cosines[index];
}

// circular value : [0..1024] corresponds to [0..pi]
struct Fixed Tan(struct Fixed circular)
{
    int index = circular.parts.integer & 1023;
    return tans[index];
}

struct Vector2 V2Rotate(struct Vector2 v, struct Fixed angle)
{
    struct Vector2 res;
    struct Fixed circular = RadianToCircular(angle);
    struct Fixed cs = Cosine(circular);
    struct Fixed sn = Sine(circular);

    res.x = Sub(Mul(v.x, cs), Mul(v.y, sn));
    res.y = Add(Mul(v.x, sn), Mul(v.y, cs));

    return res;
}

struct Vector3 V3Rotate(struct Vector3 v, struct Fixed angle)
{
    struct Vector3 res;
    struct Fixed circular = RadianToCircular(angle);
    struct Fixed cs = Cosine(circular);
    struct Fixed sn = Sine(circular);

    res.x = Sub(Mul(v.x, cs), Mul(v.z, sn));
    res.y = v.y;
    res.z = Add(Mul(v.x, sn), Mul(v.z, cs));

    return res;
}

struct Matrix_t RotateX(struct Fixed angle)
{
    struct Matrix_t res;
    memset(res.v, 0, sizeof(struct Fixed) * 16);

    struct Fixed cs = Cosine(angle);
    struct Fixed sn = Sine(angle);
    res.v[0] = FromInt(1);
    res.v[5] = cs;
    res.v[6] = sn;
    res.v[9] = Neg(sn);
    res.v[10] = cs;
    res.v[15] = FromInt(1);
    return res;
}

struct Matrix_t RotateZ(struct Fixed angle)
{
    struct Matrix_t res;
    memset(res.v, 0, sizeof(struct Fixed) * 16);

    struct Fixed cs = Cosine(angle);
    struct Fixed sn = Sine(angle);
    res.v[0] = cs;
    res.v[1] = sn;
    res.v[4] = Neg(sn);
    res.v[5] = cs;
    res.v[10] = FromInt(1);
    res.v[15] = FromInt(1);
    return res;
}

struct Matrix_t RotateZScale(struct Fixed angle, struct Fixed scale)
{
    struct Matrix_t res;
    memset(res.v, 0, sizeof(struct Fixed) * 16);

    struct Fixed cs = Mul(Cosine(angle), scale);
    struct Fixed sn = Mul(Sine(angle), scale);
    res.v[0] = cs;
    res.v[1] = sn;
    res.v[4] = Neg(sn);
    res.v[5] = cs;
    res.v[10] = scale;
    res.v[15] = FromInt(1);
    return res;
}

#define PI 3.141592

struct Fixed arctan2(struct Fixed y, struct Fixed x)
{
    struct Fixed coeff_1 = FromFixed(PI / 4 * 65536.);
    struct Fixed coeff_2 = FromFixed(3 * coeff_1.value);
    struct Fixed abs_y = FromFixed(abs(y.value) + 1);      // kludge to prevent 0/0 condition
    struct Fixed angle;
    if (x.value >= 0)
    {
        struct Fixed r = Div(Sub(x, abs_y), Add(x, abs_y));
        angle = Sub(coeff_1, Mul(coeff_1, r));
    }
    else
    {
        struct Fixed r = Div(Add(x, abs_y), Sub(abs_y, x));
        angle = Sub(coeff_2, Mul(coeff_1, r));
    }
    if (y.value < 0)
    {
        return(Neg(angle));     // negate if in quad III or IV
    }
    else
    {
        return(angle);
    }
}


struct Fixed GetAngle(struct Vector2* posA, struct Vector2* posB)
{
    struct Vector2 offset = V2Sub(*posA, *posB);
    struct Fixed angle = arctan2(Neg(offset.y), offset.x);
    angle = RadianToCircular(angle);
    return angle;
}

struct Fixed GetAngleFromDirection(struct Vector2 direction)
{
    struct Fixed angle = arctan2(Neg(direction.y), direction.x);
    angle = RadianToCircular(angle);
    return angle;
}

struct Vector2 GetDirection(struct Vector2* posA, struct Vector2* posB)
{
    struct Vector2 direction = V2Sub(*posA, *posB);
    direction = V2Normalize(direction);
    return direction;
}
