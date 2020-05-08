
enum EnemyType
{
    EnemyTypeWhite,
    EnemyTypeBlack,
    EnemyTypeWhiteSmall,
    EnemyTypeBlackSmall,
};
struct Enemy
{
    struct Vector2 position;
    short life;
    short localTime;
    unsigned short pathIndex;
    struct Vector2 pathOffset;
    enum EnemyType enemyType;
};

#define MAX_ENEMIES 128
struct Enemy Enemies[MAX_ENEMIES];
unsigned char enemyCount = 0;

void RemoveEnemy(struct Enemy* enemyPtr)
{
    enemyCount--;
    if (enemyCount)
    {
        *enemyPtr = Enemies[enemyCount];
    }

    struct Bullet* bulletPtr = bullets;
    unsigned int enemyIndex = enemyPtr - Enemies;
    for (unsigned int i = 0; i < bulletCount; i++)
    {
        if (bulletPtr->bulletType == PlayerWhiteTrail || bulletPtr->bulletType == PlayerBlackTrail)
        {
            if (enemyCount)
            {
                if (bulletPtr->enemyIndex != INVALID_ENEMY_INDEX && bulletPtr->enemyIndex >= enemyIndex)
                {
                    bulletPtr->enemyIndex --;
                }
            }
            else
            {
                bulletPtr->enemyIndex = INVALID_ENEMY_INDEX;
            }
        }
    }
}

void SpawnEnemy(enum EnemyType enemyType, struct Vector2 pathOffset, int pathIndex)
{
    if (enemyCount >= MAX_ENEMIES -1)
    {
        return;
    }
    struct Enemy* enemy = &Enemies[enemyCount];
    //enemy->position = position;
    enemy->life = ENEMY_LIFE[enemyType];
    enemy->enemyType = enemyType;
    enemy->localTime = 0;
    enemy->pathIndex = pathIndex;
    enemy->pathOffset = pathOffset;
    enemyCount++;
}

void TickEnemies()
{
    struct Enemy* enemyPtr = Enemies;
    struct Enemy* enemyEndPtr = Enemies + enemyCount;
    while (enemyPtr < enemyEndPtr)
    {
        /*
        struct Fixed t = Mul(FromInt(enemyPtr->localTime), FromFixed(0x100));
        struct Vector2 pts[3] = {V2FromInt(330, 160), V2FromInt(100, 100), V2FromInt(110, 0)};

        enemyPtr->position = V2Lerp(V2Lerp(pts[0], pts[1], t), V2Lerp(pts[1], pts[2], t), t);
        */
        struct Path* path = &Paths[enemyPtr->pathIndex];
        
        enemyPtr->position = V2Add(path->positions[enemyPtr->localTime], enemyPtr->pathOffset);
        enemyPtr->localTime++;

        bool removeEnemy = (enemyPtr->life < 0) || (enemyPtr->localTime >= path->positionCount) || IsClipped(enemyPtr->position, -300);

        if (removeEnemy)
        {
            if (enemyPtr->life < 0)
            {
                SpawnSmallExplosion(enemyPtr->position);
                for (int i = 0; i < 3; i++)
                {
                    int random = (fastrand() & 0xF) - 8;
                    struct Fixed angle;
                    angle.value = random * 0x1AAA;
                    struct Vector2 direction = V2Rotate(V2FromInt(-1, 0), angle);
                    SpawnBullet(enemyPtr->position, direction, (enemyPtr->enemyType == EnemyTypeWhite) ? EnemySmallWhite : EnemySmallBlack);
                }
                FreezeFrame(ENEMY_FREEZE_FRAMES[enemyPtr->enemyType]);
            }
            RemoveEnemy(enemyPtr);
            enemyEndPtr--;
        }
        else
        {
            enemyPtr++;
        }
    }
}

void DrawEnemies()
{
    struct Enemy* enemyPtr = Enemies;
    struct Vector2 halfExtendEnemy;
    unsigned char enemyColor;
    for (unsigned int i = 0; i < enemyCount; i++, enemyPtr++)
    {
        switch(enemyPtr->enemyType)
        {
            case EnemyTypeWhite:
                enemyColor = 12;
                halfExtendEnemy = V2FromInt(8, 8);
                break;
            case EnemyTypeBlack:
                enemyColor = 3;
                halfExtendEnemy = V2FromInt(8, 8);
                break;
            case EnemyTypeWhiteSmall:
                enemyColor = 12;
                halfExtendEnemy = V2FromInt(4, 4);
                break;
            case EnemyTypeBlackSmall:
                enemyColor = 3;
                halfExtendEnemy = V2FromInt(4, 4);
                break;
        }
        DrawRectangle(enemyPtr->position, halfExtendEnemy, enemyColor);
    }
}