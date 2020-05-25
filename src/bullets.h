enum BulletType
{
    PlayerWhite,
    PlayerBlack,
    EnemySmallWhite,
    EnemySmallBlack,
    PlayerWhiteTrail,
    PlayerBlackTrail,
    EnemyTearWhite,
    EnemyTearBlack,
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

uint8_t ballShitWhite[4 * 16 * 16];
uint8_t ballShitBlack[4 * 16 * 16];
uint8_t rotorShitWhite[4 * 16 * 16];
uint8_t rotorShitBlack[4 * 16 * 16];
uint8_t tearShotWhite[3 * 16 * 16 * 16];
uint8_t tearShotBlack[3 * 16 * 16 * 16];


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
    if (enemyIndex == INVALID_ENEMY_INDEX)
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
    bool isTrail = false;
    for (unsigned int i = 0; i < bulletCount; i++)
    {
        switch (bulletPtr->bulletType)
        {
            case PlayerWhite:
                halfExtend = V2FromInt(16, 16);
                bulletColor = 13;
                bulletLayer = 0;
                isTrail = false;
                break;
            case PlayerBlack:
                halfExtend = V2FromInt(16, 16);
                bulletColor = 3;
                bulletLayer = 0;
                isTrail = false;
                break;
            case EnemySmallWhite:
                halfExtend = V2FromInt(6, 6);
                bulletColor = 15;
                bulletLayer = 1;
                isTrail = false;
                break;
            case EnemySmallBlack:
                halfExtend = V2FromInt(6, 6);
                bulletColor = 0;
                bulletLayer = 1;
                isTrail = false;
                break;
            case PlayerWhiteTrail:
                halfExtend = V2FromInt(3, 3);
                bulletColor = 15;
                bulletLayer = 0;
                isTrail = true;
                break;
            case PlayerBlackTrail:
                halfExtend = V2FromInt(3, 3);
                bulletColor = 0;
                bulletLayer = 0;
                isTrail = true;
                break;
        }
        if (bulletPtr->bulletType == PlayerWhite || bulletPtr->bulletType == PlayerBlack)
        {
            static const int styWhite[8] = {
                0, 8, 13, 22, 33, 39, 45, 52
            };
            static const int lyWhite[8] = {
                7, 5, 7, 8, 4, 5, 6, 7
            };
            static const int styBlack[8] = {
                1, 8, 13, 22, 28, 35, 39, 49
            };
            static const int lyBlack[8] = {
                5, 3, 7, 4, 5, 2, 8, 4
            };
            const int* sty = styBlack;
            const int* ly = lyBlack;
            extern uint8_t* remappedShootWhite;
            extern uint8_t* remappedShootBlack;
            uint8_t* remappedShoot = remappedShootBlack;
            if (bulletPtr->bulletType == PlayerWhite)
            {
                const int* sty = styWhite;
                const int* ly = lyWhite;
                remappedShoot = remappedShootWhite;
            }
            static int shotIt = 0;
            int bindex = shotIt&7;
            shotIt++;
            
            struct Vector2 b1 = bulletPtr->position;
            struct Vector2 b2 = bulletPtr->position;
            b1.y = Sub(b1.y, FromInt(5));
            b2.y = Add(b2.y, FromInt(5));
            DrawSprite(b1, remappedShoot + sty[bindex] * 32, 32, ly[bindex], false);
            DrawSprite(b2, remappedShoot + sty[bindex] * 32, 32, ly[bindex], true);
        }
        else
        if (layer == bulletLayer)
        {
            if (isTrail)
            {
                int trailPosCount = min(bulletPtr->life, 8);
                for (int i = 0; i < trailPosCount - 1; i++)
                {
                    //DrawCircle(bulletPtr->trailPositions[i], halfExtend.x.integer, 0, bulletColor);
                    struct Vector2 p0 = bulletPtr->trailPositions[i];
                    struct Vector2 p1 = bulletPtr->trailPositions[i + 1];
                    DrawLine(p0.x.integer, p0.y.integer, p1.x.integer, p1.y.integer, bulletColor);
                }
            }
            else
            {
                uint8_t *sprite = (bulletPtr->bulletType&1) ? ballShitBlack : ballShitWhite;
                //uint8_t* sprite = (bulletPtr->bulletType & 1) ? rotorShitBlack : rotorShitWhite;
                int index = ((bulletPtr->life + i) >> 2) & 3;
                DrawSprite(bulletPtr->position, &sprite[index * 16 * 16], 16, 16, false);
            }
        }
        bulletPtr++;
    }
}