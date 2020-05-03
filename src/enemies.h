
enum EnemyType
{
    EnemyTypeWhite,
    EnemyTypeBlack,
};
struct Enemy
{
    struct Vector2 position;
    short life;
    short localTime;
    unsigned short pathIndex;
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
}

void SpawnEnemy(struct Vector2 position, enum EnemyType enemyType)
{
    if (enemyCount >= MAX_ENEMIES -1)
    {
        return;
    }
    struct Enemy* enemy = &Enemies[enemyCount];
    enemy->position = position;
    enemy->life = 48;
    enemy->enemyType = enemyType;
    enemy->localTime = 0;
    enemy->pathIndex = 0;
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
        
        enemyPtr->position = path->positions[enemyPtr->localTime];
        enemyPtr->localTime++;

        bool removeEnemy = (enemyPtr->life < 0) || (enemyPtr->localTime >= path->positionCount) || IsClipped(enemyPtr->position, -30);

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
                FreezeFrame(3);
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
    struct Vector2 halfExtendEnemy;
    V2SetInt(&halfExtendEnemy, 8, 8);
    struct Enemy* enemyPtr = Enemies;
    unsigned char bulletColor = 0;
    for (unsigned int i = 0; i < enemyCount; i++)
    {
        unsigned char enemyColor = (enemyPtr->enemyType == EnemyTypeWhite) ? 12 : 3;
        Rectangle(enemyPtr->position, halfExtendEnemy, enemyColor);

        enemyPtr++;
    }
}