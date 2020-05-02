enum BulletType
{
    PlayerWhite,
    PlayerBlack,
};

struct Bullet
{
    struct Vector2 position;
    struct Vector2 direction;
    enum BulletType bulletType;
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

void TickBullets()
{
    struct Bullet* bulletPtr = bullets;
    struct Bullet* bulletEndPtr = bullets + bulletCount;
    while(bulletPtr < bulletEndPtr)
    {
        bulletPtr->position = V2Add(bulletPtr->position, bulletPtr->direction);
        bool removeBullet = IsClipped(bulletPtr->position, -16);

        if (removeBullet)
        {
            RemoveBullet(bulletPtr);
            bulletEndPtr--;
        }
        else
        {
            bulletPtr++;
        }
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

void DrawBullets()
{
    struct Vector2 halfExtendPlayerBullet;
    V2SetInt(&halfExtendPlayerBullet, 16, 16);
    struct Vector2* halfExtendPtr;
    struct Bullet* bulletPtr = bullets;
    unsigned char bulletColor = 0;
    for (unsigned int i = 0; i < bulletCount; i++)
    {
        switch (bulletPtr->bulletType)
        {
            case PlayerWhite:
                halfExtendPtr = &halfExtendPlayerBullet;
                bulletColor = 15;
                break;
            case PlayerBlack:
                halfExtendPtr = &halfExtendPlayerBullet;
                bulletColor = 1;
                break;

        }
        Rectangle(bulletPtr->position, *halfExtendPtr, bulletColor);

        bulletPtr++;
    }
}