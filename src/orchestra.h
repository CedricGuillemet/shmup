
int GlobalFrame = 0;
int enemySpawnCount = 0;
int GlobalSpawnIndex = 0;

void ComputePathBase(struct Vector2* steps, struct Vector2* positions, int positionCount, int pointCount, struct Vector2 offset)
{
    struct Vector2* step = steps;
    int segmentCount = (positionCount - 1) / 2;
    int segmentStepCount = pointCount / segmentCount;
    struct Fixed speed = Div(FromInt(1), FromInt(segmentStepCount));
    for (int seg = 0; seg < segmentCount; seg++)
    {
        for (int i = 0; i < segmentStepCount; i++)
        {
            struct Fixed t = Mul(FromInt(i), speed);
            *step++ = V2Add(offset, V2Lerp(V2Lerp(positions[seg*2], positions[seg*2 + 1], t), V2Lerp(positions[seg*2 + 1], positions[seg*2 + 2], t), t));
        }
    }
}

void ComputePath(int pathIndex, struct Vector2* positions, int positionCount, int pointCount)
{
    Paths[pathIndex] = NewPath(pointCount);
    struct Path* path = &Paths[pathIndex];
    ComputePathBase(path->positions, positions, positionCount, pointCount, V2FromInt(0, 0));
}

void ComputePathAppend(int pathIndex, struct Vector2* positions, int positionCount, int pointCount, int frameOffset)
{
    struct Path* path = &Paths[pathIndex];
    ReallocPath(path, frameOffset + pointCount);

    ComputePathBase(path->positions + frameOffset, positions, positionCount, pointCount, *(path->positions + frameOffset - 1));
}

void PrecomputePaths()
{
    struct Vector2 pts0[3] = { V2FromInt(0, 0), V2FromInt(-200, -30), V2FromInt(-280, 200) };
    ComputePath(0, pts0, 3, 256);

    struct Vector2 pts1[3] = { V2FromInt(0, 0), V2FromInt(-200, 30), V2FromInt(-280, -200) };
    ComputePath(1, pts1, 3, 256);

    //
    struct Vector2 pts2[3] = { V2FromInt(0, -20), V2FromInt(-150, -140), V2FromInt(-150, 140) };
    ComputePath(2, pts2, 3, 256);

    for (int i = 3; i < 9; i++)
    {
        Paths[i] = ClonePath(&Paths[2]);
        struct Vector2 pts3[3] = { V2FromInt(0, 0), V2FromInt(-110, 0), V2FromInt(-110, -200) };
        ComputePathAppend(i, pts3, 3, 256, 240 - 0x18 * (i - 3));
    }

    for (int i = 9; i < 15; i++)
    {
        Paths[i] = CloneSymetryPath(&Paths[2]);
        struct Vector2 pts3[3] = { V2FromInt(0, 0), V2FromInt(-110, 0), V2FromInt(-110, 200) };
        ComputePathAppend(i, pts3, 3, 256, 240 - 0x18 * (i - 9));
    }

    struct Vector2 pts3[7] = { V2FromInt(0, 0), V2FromInt(-80, 0), V2FromInt(-80, 70), V2FromInt(-80, 120), V2FromInt(-180, 120), V2FromInt(-240, 120), V2FromInt(-240, -80) };
    ComputePath(15, pts3, 7, 512);

    Paths[16] = CloneSymetryPath(&Paths[15]);
}

struct Spawn
{
    int frame;
    enum EnemyType enemyType;
    struct Vector2 pathOffset;
    int pathIndex;
};

struct Spawn Spawns[] = {
    


    { -0x80, EnemyTypeWhite, V2INT(330, 80), 0},
    {  0x20, EnemyTypeWhite, V2INT(330, 80), 0},
    {  0x20, EnemyTypeWhite, V2INT(330, 80), 0},
    {  0x20, EnemyTypeWhite, V2INT(330, 80), 0},
    {  0x20, EnemyTypeWhite, V2INT(330, 80), 0},
    {  0x20, EnemyTypeWhite, V2INT(330, 80), 0},

    {-0x110, EnemyTypeBlack, V2INT(330, 120), 1},
    {  0x20, EnemyTypeBlack, V2INT(330, 120), 1},
    {  0x20, EnemyTypeBlack, V2INT(330, 120), 1},
    {  0x20, EnemyTypeBlack, V2INT(330, 120), 1},
    {  0x20, EnemyTypeBlack, V2INT(330, 120), 1},
    {  0x20, EnemyTypeBlack, V2INT(330, 120), 1},
    
    {-0x200, EnemyTypeWhite, V2INT(330, 80), 3},
    {  0x20, EnemyTypeWhite, V2INT(330, 80), 4},
    {  0x20, EnemyTypeWhite, V2INT(330, 80), 5},
    {  0x20, EnemyTypeWhite, V2INT(330, 80), 6},
    {  0x20, EnemyTypeWhite, V2INT(330, 80), 7},
    {  0x20, EnemyTypeWhite, V2INT(330, 80), 8},

    {-0x280, EnemyTypeBlack, V2INT(330, 80), 9},
    {  0x20, EnemyTypeBlack, V2INT(330, 80), 10},
    {  0x20, EnemyTypeBlack, V2INT(330, 80), 11},
    {  0x20, EnemyTypeBlack, V2INT(330, 80), 12},
    {  0x20, EnemyTypeBlack, V2INT(330, 80), 13},
    {  0x20, EnemyTypeBlack, V2INT(330, 80), 14},
    


    {-0x400, EnemyTypeWhiteSmall, V2INT(330, 20), 15},
    {  0x02, EnemyTypeWhiteSmall, V2INT(350, 20), 15},
    {  0x03, EnemyTypeWhiteSmall, V2INT(370, 20), 15},

    {  0x03, EnemyTypeWhiteSmall, V2INT(330, 40), 15},
    {  0x01, EnemyTypeWhiteSmall, V2INT(350, 40), 15},
    {  0x03, EnemyTypeWhiteSmall, V2INT(370, 40), 15},

    {  0x04, EnemyTypeWhiteSmall, V2INT(330, 60), 15},
    {  0x02, EnemyTypeWhiteSmall, V2INT(350, 60), 15},
    {  0x00, EnemyTypeWhiteSmall, V2INT(370, 60), 15},

    {-0x410, EnemyTypeWhiteSmall, V2INT(390, 20), 15},
    {  0x01, EnemyTypeWhiteSmall, V2INT(410, 20), 15},
    {  0x04, EnemyTypeWhiteSmall, V2INT(430, 20), 15},

    {  0x01, EnemyTypeWhiteSmall, V2INT(390, 40), 15},
    {  0x03, EnemyTypeWhiteSmall, V2INT(410, 40), 15},
    {  0x02, EnemyTypeWhiteSmall, V2INT(430, 40), 15},

    {  0x00, EnemyTypeWhiteSmall, V2INT(390, 60), 15},
    {  0x03, EnemyTypeWhiteSmall, V2INT(410, 60), 15},
    {  0x02, EnemyTypeWhiteSmall, V2INT(430, 60), 15},

    {-0x410, EnemyTypeBlackSmall, V2INT(330, 180), 16},
    {  0x02, EnemyTypeBlackSmall, V2INT(350, 180), 16},
    {  0x03, EnemyTypeBlackSmall, V2INT(370, 180), 16},

    {  0x03, EnemyTypeBlackSmall, V2INT(330, 160), 16},
    {  0x01, EnemyTypeBlackSmall, V2INT(350, 160), 16},
    {  0x03, EnemyTypeBlackSmall, V2INT(370, 160), 16},

    {  0x04, EnemyTypeBlackSmall, V2INT(330, 140), 16},
    {  0x02, EnemyTypeBlackSmall, V2INT(350, 140), 16},
    {  0x00, EnemyTypeBlackSmall, V2INT(370, 140), 16},

    {-0x420, EnemyTypeBlackSmall, V2INT(390, 180), 16},
    {  0x01, EnemyTypeBlackSmall, V2INT(410, 180), 16},
    {  0x04, EnemyTypeBlackSmall, V2INT(430, 180), 16},

    {  0x01, EnemyTypeBlackSmall, V2INT(390, 160), 16},
    {  0x03, EnemyTypeBlackSmall, V2INT(410, 160), 16},
    {  0x02, EnemyTypeBlackSmall, V2INT(430, 160), 16},

    {  0x00, EnemyTypeBlackSmall, V2INT(390, 140), 16},
    {  0x03, EnemyTypeBlackSmall, V2INT(410, 140), 16},
    {  0x02, EnemyTypeBlackSmall, V2INT(430, 140), 16},
};

int sortSpawn(void *a, void *b)
{
    struct Spawn* spawnA = (struct Spawn*)a;
    struct Spawn* spawnB = (struct Spawn*)b;
    if (spawnA->frame < spawnB->frame)
    {
        return -1;
    }
    if (spawnA->frame > spawnB->frame)
    {
        return 1;
    }
    return 0;
}

void PrecomputeSpawns()
{
    struct Spawn* spawn = Spawns;
    int spawnCount = sizeof(Spawns) / sizeof(struct Spawn);
    for (int i = 0;i < spawnCount; i++, spawn++)
    {
        if (spawn->frame < 0)
        {
            spawn->frame = -spawn->frame;
        }
        else
        {
            spawn->frame += (spawn - 1)->frame;
        }
    }
    qsort(Spawns, spawnCount, sizeof(struct Spawn), sortSpawn);
}

void TickOrchestra()
{
    if (GlobalSpawnIndex < sizeof(Spawns)/sizeof(struct Spawn))
    {
        while (GlobalFrame == Spawns[GlobalSpawnIndex].frame)
        {
            struct Spawn* toSpawn = &Spawns[GlobalSpawnIndex];
            SpawnEnemy(toSpawn->enemyType, toSpawn->pathOffset, toSpawn->pathIndex);
            GlobalSpawnIndex++;
        }
        GlobalFrame++;
    }
}
