
int GlobalFrame = 0;
int enemySpawnCount = 0;


void PrecomputePaths()
{
    Paths[0] = NewPath(256);
    for (int i = 0; i < 256; i++)
    {
        struct Fixed t = Mul(FromInt(i), FromFixed(0x100));
        struct Vector2 pts[3] = { V2FromInt(330, 160), V2FromInt(100, 100), V2FromInt(110, 0) };

        Paths[0].positions[i] = V2Lerp(V2Lerp(pts[0], pts[1], t), V2Lerp(pts[1], pts[2], t), t);
    }
}

void TickOrchestra()
{
    /*
    if ((GlobalFrame & 0x7F) == 0x7F)
    {
        SpawnEnemy(V2FromInt(330, ((enemySpawnCount & 3) == 0) ? 140 : 60), (enemySpawnCount & 1)? EnemyTypeWhite : EnemyTypeBlack, V2FromInt(0, -30));
        SpawnEnemy(V2FromInt(330, ((enemySpawnCount & 3) == 0) ? 140 : 60), (enemySpawnCount & 1) ? EnemyTypeWhite : EnemyTypeBlack, V2FromInt(5, -60));
        SpawnEnemy(V2FromInt(330, ((enemySpawnCount & 3) == 0) ? 140 : 60), (enemySpawnCount & 1) ? EnemyTypeWhite : EnemyTypeBlack, V2FromInt(10, -90));
        enemySpawnCount++;
    }
    */

    if (GlobalFrame == 0x80)
    {
        SpawnEnemy(V2FromInt(330, ((enemySpawnCount & 3) == 0) ? 140 : 60), (enemySpawnCount & 1) ? EnemyTypeWhite : EnemyTypeBlack, V2FromInt(0, -30));
        enemySpawnCount++;
    }
    if (GlobalFrame == 0xA0)
    {
        SpawnEnemy(V2FromInt(330, ((enemySpawnCount & 3) == 0) ? 140 : 60), (enemySpawnCount & 1) ? EnemyTypeWhite : EnemyTypeBlack, V2FromInt(0, -60));
        enemySpawnCount++;
    }
    if (GlobalFrame == 0xC0)
    {
        SpawnEnemy(V2FromInt(330, ((enemySpawnCount & 3) == 0) ? 140 : 60), (enemySpawnCount & 1) ? EnemyTypeWhite : EnemyTypeBlack, V2FromInt(0, -90));
        enemySpawnCount++;
    }

    GlobalFrame++;
}