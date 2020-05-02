
int GlobalFrame = 0;
int enemySpawnCount = 0;

void TickOrchestra()
{
    if (!(GlobalFrame & 0x7F))
    {
        SpawnEnemy(V2FromInt(330, ((enemySpawnCount & 3) == 0) ? 140 : 60), (enemySpawnCount & 1)? EnemyTypeWhite : EnemyTypeBlack);
        enemySpawnCount++;
    }

    GlobalFrame++;
}