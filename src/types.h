
// Fixed point value

struct Fixed
{
    union 
    {
        int value;
        struct parts
        {
            unsigned short decimal; // int
            short integer; // fixed point
        };
    };
};

struct Fixed Add(struct Fixed a, struct Fixed b)
{
    struct Fixed res;
    res.value = a.value + b.value;
    return res;
}

struct Fixed Sub(struct Fixed a, struct Fixed b)
{
    struct Fixed res;
    res.value = a.value - b.value;
    return res;
}

struct Fixed Mul(struct Fixed a, struct Fixed b)
{
    struct Fixed res;
    __asm {
        mov eax, a.value
        imul b.value;
        mov res.value.integer, dx
        shr eax, 16
        mov res.value.decimal, ax
    }
    return res;
}

struct Fixed Div(struct Fixed a, struct Fixed b)
{
    struct Fixed res;
    __asm {
        xor eax, eax
        xor edx, edx
        mov ax, a.value.integer
        cwd 
        shl edx,16
        mov dx, ax
        mov ax, a.value.decimal
        shl eax, 16
        //xchg eax,edx
        idiv b.value
        mov res.value, eax
    }
    //res.value = a.value / b.value;
    return res;
}

struct Fixed FromInt(int v)
{
    struct Fixed res;
    res.integer = v;
    res.decimal = 0;
    return res;
}

void SetInt(struct Fixed* value, int v)
{
    value->integer = v;
    value->decimal = 0;
}

void SetFixed(struct Fixed* value, int i)
{
    value->value = i;
}

struct Fixed FromFixed(int i)
{
    struct Fixed res;
    res.value = i;
    return res;
}

struct Fixed Neg(struct Fixed value)
{
    struct Fixed res;
    res.value = -value.value;
    return res;
}

struct Fixed Lerp(struct Fixed a, struct Fixed b, struct Fixed t)
{
    return Add(a, Mul(Sub(b, a), t));
}

// Vector2

struct Vector2
{
    struct Fixed x;
    struct Fixed y;
};

void V2SetInt(struct Vector2* value, int x, int y)
{
    SetInt(&value->x, x);
    SetInt(&value->y, y);
}

void V2SetFixed(struct Vector2* value, struct Fixed x, struct Fixed y)
{
    value->x = x;
    value->y = y;
}

struct Vector2 V2Add(struct Vector2 a, struct Vector2 b)
{
    struct Vector2 res;

    res.x = Add(a.x, b.x);
    res.y = Add(a.y, b.y);

    return res;
}

struct Vector2 V2Sub(struct Vector2 a, struct Vector2 b)
{
    struct Vector2 res;

    res.x = Sub(a.x, b.x);
    res.y = Sub(a.y, b.y);

    return res;
}

struct Vector2 V2Normalize(struct Vector2 v)
{
    struct Vector2 res;
    struct Fixed len;

    if (abs(v.x.integer) > 128 || abs(v.y.integer) > 128)
    {
        v.x.value >>= 1;
        v.y.value >>= 1;
    }
    len = Mul(v.x, v.x);
    len = Add(len, Mul(v.y, v.y));
    len.value = (int)(sqrt(len.value)) << 8;
    res.x = Div(v.x, len);
    res.y = Div(v.y, len);

    return res;
}

struct Fixed V2LengthSq(struct Vector2 v)
{
    struct Fixed lenSq;

    lenSq = Mul(v.x, v.x);
    lenSq = Add(lenSq, Mul(v.y, v.y));

    return lenSq;
}

struct Vector2 V2FromInt(int x, int y)
{
    struct Vector2 res;

    SetInt(&res.x, x);
    SetInt(&res.y, y);

    return res;
}

struct Vector2 V2Lerp(struct Vector2 a, struct Vector2 b, struct Fixed t)
{
    struct Vector2 res;

    res.x = Lerp(a.x, b.x, t);
    res.y = Lerp(a.y, b.y, t);

    return res;
}

struct Vector2 V2Mul(struct Vector2 v, struct Fixed m)
{
    struct Vector2 res;

    res.x = Mul(v.x, m);
    res.y = Mul(v.y, m);

    return res;
}

// Vector3

struct Vector3
{
    struct Fixed x, y, z;
};

struct Vector3 V3Add(struct Vector3 a, struct Vector3 b)
{
    struct Vector3 res;

    res.x = Add(a.x, b.x);
    res.y = Add(a.y, b.y);
    res.z = Add(a.z, b.z);

    return res;
}

struct Vector3 V3Sub(struct Vector3 a, struct Vector3 b)
{
    struct Vector3 res;

    res.x = Sub(a.x, b.x);
    res.y = Sub(a.y, b.y);
    res.z = Sub(a.z, b.z);

    return res;
}

struct Vector3 V3Mul(struct Vector3 v, struct Fixed m)
{
    struct Vector3 res;

    res.x = Mul(v.x, m);
    res.y = Mul(v.y, m);
    res.z = Mul(v.z, m);

    return res;
}

struct Vector3 V3Normalize(struct Vector3 v)
{
    struct Vector3 res;
    struct Fixed len;

    len = Mul(v.x, v.x);
    len = Add(len, Mul(v.y, v.y));
    len = Add(len, Mul(v.z, v.z));
    len.value = (int)(sqrt(len.value)) << 8;
    res.x = Div(v.x, len);
    res.y = Div(v.y, len);
    res.z = Div(v.z, len);

    return res;
}

struct Fixed V3Dot(struct Vector3 a, struct Vector3 b)
{
    struct Fixed res;

    res = Mul(a.x, b.x);
    res = Add(res, Mul(a.y, b.y));
    res = Add(res, Mul(a.z, b.z));

    return res;
}

struct Vector3 V3Cross(struct Vector3 a, struct Vector3 b)
{
    struct Vector3 res;

    res.x = Sub(Mul(a.y, b.z), Mul(a.z, b.y));
    res.y = Sub(Mul(a.z, b.x), Mul(a.x, b.z));
    res.z = Sub(Mul(a.x, b.y), Mul(a.y, b.x));

    return res;
}

struct Vector3 V3FromInt(int x, int y, int z)
{
    struct Vector3 res;

    SetInt(&res.x, x);
    SetInt(&res.y, y);
    SetInt(&res.z, z);

    return res;
}

struct Vector3 V3FromFixed(struct Fixed x, struct Fixed y, struct Fixed z)
{
    struct Vector3 res;

    res.x = x;
    res.y = y;
    res.z = z;

    return res;
}

struct Vector3 V3FromChar(char x, char y, char z)
{
    struct Vector3 res;

    SetInt(&res.x, (int)x);
    SetInt(&res.y, (int)y);
    SetInt(&res.z, (int)z);

    return res;
}

struct Vector3 V3Neg(struct Vector3 value)
{
    struct Vector3 res;
    res.x = Neg(value.x);
    res.y = Neg(value.y);
    res.z = Neg(value.z);
    return res;
}

// Matrix

struct Matrix_t
{
    struct Fixed v[16];
};

struct Matrix_t IdentityMatrix()
{
    struct Matrix_t res;
    memset(res.v, 0, sizeof(struct Fixed) * 16);
    for (int i = 0; i < 4; i++)
    {
        res.v[i * 5] = FromInt(1);
    }
    return res;
}

// Radian/Circular value

struct Fixed RadianToCircular(struct Fixed radian)
{
    return Mul(radian, FromFixed(21361414)); // /3.141592 * 1024 * 65536
}

int g_seed = 17;

int fastrand()
{
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}