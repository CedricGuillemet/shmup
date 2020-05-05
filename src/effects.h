
struct Explosion
{
    struct Vector2 position;
    unsigned char life;
    unsigned char ttl;
    unsigned char radiusOutStart;
    unsigned char radiusOutEnd;
    unsigned char radiusInStart;
    unsigned char radiusInEnd;
    unsigned char color;
};

#define MAX_EXPLOSIONS 32
struct Explosion Explosions[MAX_EXPLOSIONS];
unsigned int explosionCount = 0;

// t : [0..1] maps to [0..65536];
int IntLerp(int a, int b, int t)
{
    return a + (((b - a) * t) >> 16);
}

struct Explosion* AddExplosion(struct Vector2 position)
{
    if (explosionCount >= MAX_EXPLOSIONS - 1)
    {
        return NULL;
    }
    struct Explosion* res = &Explosions[explosionCount++];
    res->position = position;
    return res;
}

void RemoveExplosion(struct Explosion* ptr)
{
    explosionCount--;
    if (explosionCount)
    {
        *ptr = Explosions[explosionCount];
    }
}

void SpawnPlayerExplosion(struct Vector2 position)
{
    struct Explosion* explosion = AddExplosion(position);
    if (!explosion)
    {
        return;
    }
    explosion->life = 0;
    explosion->ttl = 60;
    explosion->radiusInStart = 0;
    explosion->radiusOutStart = 16;
    explosion->radiusInEnd = 128;
    explosion->radiusOutEnd = 128;
    explosion->color = 19;
}

void SpawnSmallExplosion(struct Vector2 position)
{
    struct Explosion* explosion = AddExplosion(position);
    if (!explosion)
    {
        return;
    }
    explosion->life = 0;
    explosion->ttl = 30;
    explosion->radiusInStart = 0;
    explosion->radiusOutStart = 8;
    explosion->radiusInEnd = 32;
    explosion->radiusOutEnd = 32;
    explosion->color = 19;
}

void SpawnTinyExplosion(struct Vector2 position)
{
    struct Explosion* explosion = AddExplosion(position);
    if (!explosion)
    {
        return;
    }
    explosion->life = 0;
    explosion->ttl = 14;
    explosion->radiusInStart = 0;
    explosion->radiusOutStart = 6;
    explosion->radiusInEnd = 16;
    explosion->radiusOutEnd = 16;
    explosion->color = 19;
}

void SpawnRandomTinyExplosion(struct Vector2 position)
{
    position.x.integer += (fastrand() & 0xF) - 15;
    position.y.integer += (fastrand() & 0xF) - 7;
    SpawnTinyExplosion(position);
}

void TickExplosions()
{
    struct Explosion* ptr = Explosions;
    struct Explosion* endPtr = Explosions + explosionCount;
    while (ptr < endPtr)
    {
        ptr->life ++;
        if (ptr->life >= ptr->ttl)
        {
            RemoveExplosion(ptr);
            endPtr--;
        }
        else
        {
            ptr++;
        }
    }
}

void TickEffects()
{
    TickExplosions();
}

void DrawExplosions()
{
    struct Explosion* ptr = Explosions;
    struct Explosion* endPtr = Explosions + explosionCount;
    while (ptr < endPtr)
    {
        int t = (ptr->life << 16) / ptr->ttl;
        DrawCircle(ptr->position, IntLerp(ptr->radiusOutStart, ptr->radiusOutEnd, t), IntLerp(ptr->radiusInStart, ptr->radiusInEnd, t), ptr->color);
        ptr++;
    }
}

void DrawEffects()
{
    DrawExplosions();
}

