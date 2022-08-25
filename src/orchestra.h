
int GlobalFrame = 0;
int GlobalSpawnFrame = 0;
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
            struct Vector2 value = V2Add(offset, V2Lerp(V2Lerp(positions[seg*2], positions[seg*2 + 1], t), V2Lerp(positions[seg*2 + 1], positions[seg*2 + 2], t), t));
            *step++ = value;
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


    struct Vector2 pts4[7] = { V2FromInt(0, 0), V2FromInt(-100, 100), V2FromInt(-110, 100), V2FromInt(-120, 100), V2FromInt(-130, 100), V2FromInt(-140, 100), V2FromInt(-250, -100) };
    ComputePath(17, pts4, 7, 384);
    Paths[18] = CloneSymetryPath(&Paths[17]);

    // batch of 3 coming, shooting, going back
    struct Vector2 pts5[9] = { V2FromInt(0, 0), V2FromInt(-100, 80), V2FromInt(-100, 60), V2FromInt(-105, 58), V2FromInt(-98, 62), V2FromInt(-103, 61), V2FromInt(-95, 57), V2FromInt(-80, 50), V2FromInt(0, 0) };
    ComputePath(19, pts5, 9, 384);
    Paths[20] = CloneSymetryPath(&Paths[19]);

    // going from top to down
    struct Vector2 pts6[3] = { V2FromInt(0, 0), V2FromInt(-20, 80), V2FromInt(-350, 80)};
    ComputePath(21, pts6, 3, 256);
    Paths[22] = CloneSymetryPath(&Paths[21]);

    // sliding left to right/ top to bottom
    struct Vector2 pts7[7] = { V2FromInt(0, 0), V2FromInt(-100, 100), V2FromInt(-150, 0), V2FromInt(-200, -100), V2FromInt(-250, 0), V2FromInt(-300, 100), V2FromInt(-340, 80) };
    ComputePath(23, pts7, 7, 384);
    Paths[24] = CloneSymetryPath(&Paths[23]);

    // back to front
    struct Vector2 pts8[3] = { V2FromInt(-100, 0), V2FromInt(-100, 1), V2FromInt(-100, 0) };
    ComputePath(25, pts8, 3, 1024);


    struct Vector2 pts9[3] = { V2FromInt(32, 0), V2FromInt(0, 1), V2FromInt(32, 0) };
    ComputePath(26, pts9, 3, 1024);

    // train moving in rectangle Part 1
    int ofsy = 25;
    struct Vector2 pts10[27] = { V2FromInt(0, 100 - ofsy), V2FromInt(-160, 100 - ofsy), V2FromInt(-300, 100 - ofsy) // 0
        , V2FromInt(-310, 100 - ofsy), V2FromInt(-310, 90 - ofsy) // 1
        , V2FromInt(-310, 0), V2FromInt(-310, -90 + ofsy) // 2
        , V2FromInt(-310, -100 + ofsy), V2FromInt(-300, -100 + ofsy) // 3
        , V2FromInt(-160, -100 + ofsy), V2FromInt(-90, -100 + ofsy) // 4
        , V2FromInt(-80, -100 + ofsy), V2FromInt(-80, -90 + ofsy) // 5

        , V2FromInt(-80, 0), V2FromInt(-80, 90 - ofsy) // 6
        , V2FromInt(-80, 100 - ofsy), V2FromInt(-90, 100 - ofsy) // 7

        , V2FromInt(-160, 100 - ofsy), V2FromInt(-300, 100 - ofsy) // 8
        , V2FromInt(-310, 100 - ofsy), V2FromInt(-310, 90 - ofsy) // 9
        , V2FromInt(-310, 0), V2FromInt(-310, -90 + ofsy) // 10
        , V2FromInt(-310, -100 + ofsy), V2FromInt(-300, -100 + ofsy) // 11

        , V2FromInt(-0, -100 + ofsy), V2FromInt(20, -100 + ofsy) //12

    };
    ComputePath(27, pts10, 27, 1024);
    Paths[28] = CloneSymetryPath(&Paths[27]);

}


struct Shoot
{
    int frameStart;
    int frameEnd;
    enum BulletType type;
    bool align;
};

struct Spawn
{
    int frame;
    enum EnemyType enemyType;
    struct Vector2 pathOffset;
    int pathIndex;
    unsigned char circularOffset;
    unsigned char circularRadius;
    struct Shoot shoot;
};

void TearShot(struct Enemy* enemy)
{
    struct Vector2 direction = GetDirection(&Ship.position, &enemy->position);
    struct Fixed angle = GetAngleFromDirection(direction);

    struct Bullet* bullet = SpawnBullet(enemy->position, V2Mul(direction, FromFixed(0x10000)), (enemy->enemyType&1) ? EnemyTearBlack : EnemyTearWhite);
    bullet->directionAngle16 = 15 - ((angle.parts.integer * 16 / 2048) & 15);
}

void TearShotCircular(struct Enemy* enemy)
{
    for (int i = 0; i < 32; i++)
    {
        struct Fixed tr = FromInt(i * 2048 / 32);
        struct Vector2 direction = V2FromFixed(Cosine(tr), Sine(tr));

        struct Fixed angle = GetAngleFromDirection(direction);

        struct Bullet* bullet = SpawnBullet(V2FromInt(200,100), V2Mul(direction, FromFixed(0x10000)), EnemyTearBlack);
        bullet->directionAngle16 = 15 - ((angle.parts.integer * 16 / 2048) & 15);
    }
}

struct Spawn Spawns[] = {
    { -0x080, EnemyTypeWhiteHunter, V2INT(330,   0), 19, 0, 0,   60, 70, EnemyTearWhite, true},
    {   0x04, EnemyTypeWhiteHunter, V2INT(355, -25), 19, 0, 0,   60, 70, EnemyTearWhite, true},
    {   0x02, EnemyTypeWhiteHunter, V2INT(355,  25), 19, 0, 0,   60, 70, EnemyTearWhite, true},

    
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

    // 2 banks of smalls
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

    // Batch of 3 comming and going
    { -0x580, EnemyTypeWhiteHunter, V2INT(330, 40), 17},
    {   0x02, EnemyTypeWhiteHunter, V2INT(355, 65), 17},
    {   0x01, EnemyTypeWhiteHunter, V2INT(355, 15), 17},

    { -0x700, EnemyTypeWhiteHunter, V2INT(330, (200 - 40)), 18},
    {   0x02, EnemyTypeWhiteHunter, V2INT(355, (200 - 65)), 18},
    {   0x01, EnemyTypeWhiteHunter, V2INT(355, (200 - 15)), 18},

    { -0x800, EnemyTypeBlackHunter, V2INT(330, 40), 17},
    {   0x02, EnemyTypeBlackHunter, V2INT(355, 65), 17},
    {   0x01, EnemyTypeBlackHunter, V2INT(355, 15), 17},

    { -0x880, EnemyTypeBlackHunter, V2INT(330, (200 - 40)), 18},
    {   0x02, EnemyTypeBlackHunter, V2INT(355, (200 - 65)), 18},
    {   0x01, EnemyTypeBlackHunter, V2INT(355, (200 - 15)), 18},

    { -0x900, EnemyTypeWhiteHunter, V2INT(330, 40), 17},
    {   0x02, EnemyTypeWhiteHunter, V2INT(355, 65), 17},
    {   0x01, EnemyTypeWhiteHunter, V2INT(355, 15), 17},

    { -0x980, EnemyTypeBlackHunter, V2INT(330, (200 - 40)), 18},
    {   0x02, EnemyTypeBlackHunter, V2INT(355, (200 - 65)), 18},
    {   0x01, EnemyTypeBlackHunter, V2INT(355, (200 - 15)), 18},

    // 2 batches of 2 comming, shooting and going
    {-0xA80, EnemyTypeWhiteHunter, V2INT(330,   0), 19},
    {   0x04, EnemyTypeWhiteHunter, V2INT(355, -25), 19},
    {   0x02, EnemyTypeWhiteHunter, V2INT(355,  25), 19},

    {-0xB40, EnemyTypeBlackHunter, V2INT(330, (200 -  0)), 20},
    {   0x04, EnemyTypeBlackHunter, V2INT(355, (200 + 25)), 20},
    {   0x02, EnemyTypeBlackHunter, V2INT(355, (200 - 25)), 20},

    {-0xC00, EnemyTypeWhiteHunter, V2INT(330,   0), 19},
    {   0x04, EnemyTypeWhiteHunter, V2INT(355, -25), 19},
    {   0x02, EnemyTypeBlackHunter, V2INT(355,  25), 19},

    {-0xC01, EnemyTypeBlackHunter, V2INT(330, (200 - 0)), 20},
    {   0x04, EnemyTypeBlackHunter, V2INT(355, (200 + 25)), 20},
    {   0x02, EnemyTypeWhiteHunter, V2INT(355, (200 - 25)), 20},

    // on both side + zigzag in the middle
    {-0xD80, EnemyTypeWhite, V2INT(330, 100), 23},
    {  0x80, EnemyTypeBlack, V2INT(330, 100), 24},
    {  0x60, EnemyTypeWhite, V2INT(330, 100), 23},
    {  0x40, EnemyTypeBlack, V2INT(330, 100), 24},
    {  0x30, EnemyTypeWhite, V2INT(330, 100), 23},
    {  0x30, EnemyTypeBlack, V2INT(330, 100), 24},
    {  0x30, EnemyTypeWhite, V2INT(330, 100), 23},
    {  0x30, EnemyTypeBlack, V2INT(330, 100), 24},

    {-0xE80, EnemyTypeWhite, V2INT(330, 100), 21},
    {  0x40, EnemyTypeBlack, V2INT(330, 100), 22},
    {  0x40, EnemyTypeWhite, V2INT(330, 100), 21},
    {  0x40, EnemyTypeBlack, V2INT(330, 100), 21},
    {  0x40, EnemyTypeWhite, V2INT(330, 100), 22},
    {  0x40, EnemyTypeBlack, V2INT(330, 100), 22},
    {  0x40, EnemyTypeWhite, V2INT(330, 100), 21},
    {  0x40, EnemyTypeBlack, V2INT(330, 100), 22},

    // 2 bigs with 2 satellites each
    {-0x1100, EnemyTypeWhiteBig, V2INT(360, 110), 20},
    {  0x00, EnemyTypeWhite, V2INT(360, 110), 20, 0, 32},
    {  0x00, EnemyTypeBlack, V2INT(360, 110), 20, 0x60,32},

    {-0x1110, EnemyTypeBlackBig, V2INT(360, 90), 19},
    {  0x00, EnemyTypeWhite, V2INT(360, 90), 19,0,32},
    {  0x00, EnemyTypeBlack, V2INT(360, 90), 19, 0x60,32},


    // circular and big in the middle (with beam)
    {-0x1300, EnemyTypeWhiteSmall, V2INT(360, 100), 25, 0, 64},
    { 0x11, EnemyTypeWhiteSmall, V2INT(360, 100), 25, 0, 64 },
    { 0x11, EnemyTypeWhiteSmall, V2INT(360, 100), 25, 0, 64 },

    { 0x11, EnemyTypeBlackSmall, V2INT(360, 100), 25, 0, 64 },
    { 0x11, EnemyTypeBlackSmall, V2INT(360, 100), 25, 0, 64 },
    { 0x11, EnemyTypeBlackSmall, V2INT(360, 100), 25, 0, 64 },

    { 0x11, EnemyTypeWhiteSmall, V2INT(360, 100), 25, 0, 64 },
    { 0x11, EnemyTypeWhiteSmall, V2INT(360, 100), 25, 0, 64 },
    { 0x11, EnemyTypeWhiteSmall, V2INT(360, 100), 25, 0, 64 },

    { 0x11, EnemyTypeBlackSmall, V2INT(360, 100), 25, 0, 64 },
    { 0x11, EnemyTypeBlackSmall, V2INT(360, 100), 25, 0, 64 },
    { 0x11, EnemyTypeBlackSmall, V2INT(360, 100), 25, 0, 64 },

    { -0x1300, EnemyTypeWhiteBig, V2INT(320, 100), 26 },

    // double train
    {-0x1900, EnemyTypeWhiteSmall, V2INT(320, 80), 27},
    { 0, EnemyTypeWhiteSmall, V2INT(340, 80), 27 },
    { 0, EnemyTypeWhiteSmall, V2INT(370, 80), 27 },
    { 0, EnemyTypeWhiteSmall, V2INT(390, 80), 27 },

    { 0, EnemyTypeWhiteSmall, V2INT(320, 120), 27 },
    { 0, EnemyTypeWhiteSmall, V2INT(340, 120), 27 },
    { 0, EnemyTypeWhiteSmall, V2INT(370, 120), 27 },
    { 0, EnemyTypeWhiteSmall, V2INT(390, 120), 27 },

    { -0x1900 + 0x130, EnemyTypeBlackSmall, V2INT(320, 80), 27 },
    { 0, EnemyTypeBlackSmall, V2INT(340, 80), 27 },
    { 0, EnemyTypeBlackSmall, V2INT(370, 80), 27 },
    { 0, EnemyTypeBlackSmall, V2INT(390, 80), 27 },

    { 0, EnemyTypeBlackSmall, V2INT(320, 120), 27 },
    { 0, EnemyTypeBlackSmall, V2INT(340, 120), 27 },
    { 0, EnemyTypeBlackSmall, V2INT(370, 120), 27 },
    { 0, EnemyTypeBlackSmall, V2INT(390, 120), 27 },
    
};

int sortSpawn(const void *a, const void *b)
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
        spawn->shoot.frameStart += spawn->frame;
        spawn->shoot.frameEnd += spawn->frame;
    }
    qsort(Spawns, spawnCount, sizeof(struct Spawn), sortSpawn);
}

void TickOrchestra()
{
    if (GlobalSpawnIndex < sizeof(Spawns)/sizeof(struct Spawn))
    {
        while (GlobalSpawnFrame == Spawns[GlobalSpawnIndex].frame)
        {
            struct Spawn* toSpawn = &Spawns[GlobalSpawnIndex];
            SpawnEnemy(toSpawn->enemyType, toSpawn->pathOffset, toSpawn->pathIndex, toSpawn->circularOffset, toSpawn->circularRadius, toSpawn->shoot.align);
            GlobalSpawnIndex++;
        }
        GlobalSpawnFrame++;
    }
    /*
    if (enemyCount)
    {
        if (!(GlobalFrame & 7))
        {
            TearShot(Enemies);
        }

    }
    
    if (GlobalFrame == 0)
    {
        TearShotCircular(NULL);
    }
    */
}
