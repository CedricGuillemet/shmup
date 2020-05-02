enum BulletType
{
    PlayerWhite,
    PlayerBlack,
    EnemySmallWhite,
    EnemySmallBlack,
    PlayerWhiteTrail,
    PlayerBlackTrail,
};

struct Bullet
{
    struct Vector2 position;
    struct Vector2 direction;
    enum BulletType bulletType;
    unsigned char life;
    struct Vector2 trailPositions[8];
    struct Vector2 destination;
    unsigned short enemyIndex;
};

#define MAX_BULLETS 1024
struct Bullet bullets[MAX_BULLETS];
unsigned int bulletCount = 0;

bool IsClipped(struct Vector2 position, int margin)
{
    if (position.x.integer < margin)
    {
        return true;
    }

    if (position.x.integer > SCREEN_WIDTH - margin)
    {
        return true;
    }

    if (position.y.integer < margin)
    {
        return true;
    }

    if (position.y.integer > SCREEN_HEIGHT - margin)
    {
        return true;
    }
    return false;
}

void RemoveBullet(struct Bullet* bulletPtr)
{
    bulletCount--;
    if (bulletCount)
    {
        *bulletPtr = bullets[bulletCount];
    }
}



void SpawnBullet(struct Vector2 position, struct Vector2 direction, enum BulletType bulletType)
{
    if (bulletCount >= MAX_BULLETS - 1)
    {
        return;
    }
    struct Bullet* bulletPtr = &bullets[bulletCount];
    bulletPtr->position = position;
    bulletPtr->direction = direction;
    bulletPtr->bulletType = bulletType;
    bulletCount++;
}

void SpawnTrail(struct Vector2 position, struct Vector2 direction, enum BulletType bulletType, unsigned short enemyIndex)
{
    if (bulletCount >= MAX_BULLETS - 1)
    {
        return;
    }
    struct Bullet* bulletPtr = &bullets[bulletCount];
    bulletPtr->position = position;
    bulletPtr->direction = direction;
    bulletPtr->bulletType = bulletType;
    bulletPtr->life = 0;
    bulletPtr->enemyIndex = enemyIndex;
    if (enemyIndex == 0xFFFF)
    {
        bulletPtr->destination = V2Add(position, V2FromInt(400,0));
    }
    bulletCount++;
}


void DrawBullets(int layer)
{
    struct Vector2 halfExtend;
    struct Bullet* bulletPtr = bullets;
    unsigned char bulletColor = 0;
    int bulletLayer = 0;
    bool isCircle = false;
    bool isTrail = false;
    for (unsigned int i = 0; i < bulletCount; i++)
    {
        switch (bulletPtr->bulletType)
        {
            case PlayerWhite:
                halfExtend = V2FromInt(16, 16);
                bulletColor = 13;
                bulletLayer = 0;
                isCircle = false;
                isTrail = false;
                break;
            case PlayerBlack:
                halfExtend = V2FromInt(16, 16);
                bulletColor = 3;
                bulletLayer = 0;
                isCircle = false;
                isTrail = false;
                break;
            case EnemySmallWhite:
                halfExtend = V2FromInt(6, 6);
                bulletColor = 15;
                bulletLayer = 1;
                isCircle = true;
                isTrail = false;
                break;
            case EnemySmallBlack:
                halfExtend = V2FromInt(6, 6);
                bulletColor = 0;
                bulletLayer = 1;
                isCircle = true;
                isTrail = false;
                break;
            case PlayerWhiteTrail:
                halfExtend = V2FromInt(3, 3);
                bulletColor = 15;
                bulletLayer = 0;
                isCircle = true;
                isTrail = true;
                break;
            case PlayerBlackTrail:
                halfExtend = V2FromInt(3, 3);
                bulletColor = 0;
                bulletLayer = 0;
                isCircle = true;
                isTrail = true;
                break;
        }
        if (layer == bulletLayer)
        {
            if (isTrail)
            {
                int trailPosCount = min(bulletPtr->life, 8);
                for (int i = 0; i < trailPosCount; i++)
                {
                    DrawCircle(bulletPtr->trailPositions[i], halfExtend.x.integer, 0, bulletColor);
                }
            }
            else if (isCircle)
            {
                DrawCircle(bulletPtr->position, halfExtend.x.integer, 0, bulletColor);
            }
            else
            {
                Rectangle(bulletPtr->position, halfExtend, bulletColor);
            }
        }
        bulletPtr++;
    }
}