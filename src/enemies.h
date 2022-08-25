

struct Enemy
{
    struct Vector2 position;
    short life;
    short localTime;
    unsigned short pathIndex;
    struct Vector2 pathOffset;
    enum EnemyType enemyType;
    unsigned char circularOffset;
    unsigned char circularRadius;
    bool align;
};

#define MAX_ENEMIES 128
struct Enemy Enemies[MAX_ENEMIES];
unsigned char enemyCount = 0;

void ClearEnemies()
{
    enemyCount = 0;
}

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

void SpawnEnemy(enum EnemyType enemyType, struct Vector2 pathOffset, int pathIndex, unsigned char circularOffset, unsigned char circularRadius, bool align)
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
    enemy->circularOffset = circularOffset;
    enemy->circularRadius = circularRadius;
    enemy->pathIndex = pathIndex;
    enemy->pathOffset = pathOffset;
    enemy->align = align;
    enemyCount++;
}

void TickEnemies()
{
    struct Enemy* enemyPtr = Enemies;
    struct Enemy* enemyEndPtr = Enemies + enemyCount;
    while (enemyPtr < enemyEndPtr)
    {
        struct Path* path = &Paths[enemyPtr->pathIndex];
        
        enemyPtr->position = V2Add(path->positions[enemyPtr->localTime], enemyPtr->pathOffset);
        if (enemyPtr->circularRadius > 0)
        {
            struct Fixed angle = RadianToCircular(Mul(FromInt(enemyPtr->localTime + enemyPtr->circularOffset), FromFixed(0x800)));
            struct Fixed cs = Cosine(angle);
            struct Fixed sn = Sine(angle);
            struct Vector2 rot = V2Mul(V2FromFixed(cs, sn), FromInt(enemyPtr->circularRadius));
            enemyPtr->position = V2Add(enemyPtr->position, rot);
        }
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
                    SpawnBullet(enemyPtr->position, direction, (enemyPtr->enemyType == EnemyTypeWhite) ? EnemyBallWhite : EnemyBallBlack);
                }
                FreezeFrame(ENEMY_FREEZE_FRAMES[enemyPtr->enemyType]);
                ScoreOnDestroy(enemyPtr->enemyType);
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


struct EmemyMesh
{
    char* positions;
    unsigned char* triangles;
    unsigned char* triangleColors;
    int positionCount;
    int triangleCount;
};
static const struct EmemyMesh EnemyMeshes[EnemyCount] =
{
    {enemy1Positions, enemy1Triangles, enemy1TriangleWhiteColors, sizeof(enemy1Positions) / 3, sizeof(enemy1TriangleWhiteColors)},
    {enemy1Positions, enemy1Triangles, enemy1TriangleBlackColors, sizeof(enemy1Positions) / 3, sizeof(enemy1TriangleWhiteColors)},
    {enemy0Positions, enemy0Triangles, enemy0TriangleWhiteColors, sizeof(enemy0Positions) / 3, sizeof(enemy0TriangleWhiteColors)},
    {enemy0Positions, enemy0Triangles, enemy0TriangleBlackColors, sizeof(enemy0Positions) / 3, sizeof(enemy0TriangleWhiteColors)},
    {enemy2Positions, enemy2Triangles, enemy2TriangleWhiteColors, sizeof(enemy2Positions) / 3, sizeof(enemy2TriangleWhiteColors)},
    {enemy2Positions, enemy2Triangles, enemy2TriangleBlackColors, sizeof(enemy2Positions) / 3, sizeof(enemy2TriangleWhiteColors)},
    {enemy3Positions, enemy3Triangles, enemy3TriangleWhiteColors, sizeof(enemy3Positions) / 3, sizeof(enemy3TriangleWhiteColors)},
    {enemy3Positions, enemy3Triangles, enemy3TriangleBlackColors, sizeof(enemy3Positions) / 3, sizeof(enemy3TriangleWhiteColors)},
};


void DrawEnemy(int enemyIndex, struct Vector2 position, bool align)
{
    struct Fixed scale = FromFixed(ENEMY_DISPLAY_SCALE[enemyIndex]);

    struct Matrix_t mvps;
    if (align)
    {
        struct Fixed angle = GetAngle(&position, &Ship.position);
        struct Matrix_t modelScale = RotateZScale(angle, scale);
        modelScale.v[12] = FromFixed(position.x.value / 10);
        modelScale.v[13] = FromFixed(position.y.value / 10);

        mvps = MulMatrix(modelScale, gameVP);
    }
    else
    {
        struct Matrix_t modelScale = TranslateScale(scale, scale, scale,
            FromFixed(position.x.value / 10), FromFixed(position.y.value / 10), FromInt(0));

        mvps = MulMatrix(modelScale, gameVP);
    }

    const struct EmemyMesh* enemyMesh = &EnemyMeshes[enemyIndex];

    DrawMesh(&mvps, enemyMesh->positions, enemyMesh->positionCount, enemyMesh->triangles, enemyMesh->triangleColors, enemyMesh->triangleCount);
}

void DrawEnemies()
{
    struct Enemy* enemyPtr = Enemies;
    struct Enemy* enemyPtrEnd = Enemies + enemyCount;
    while(enemyPtr < enemyPtrEnd)
    {
        DrawEnemy(enemyPtr->enemyType, enemyPtr->position, enemyPtr->align);
        enemyPtr++;
    }
}
