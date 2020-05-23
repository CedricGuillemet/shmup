
void TickBulletsDamagingEnemies()
{
    struct Bullet* bulletPtr = bullets;
    struct Bullet* bulletEndPtr = bullets + bulletCount;
    while (bulletPtr < bulletEndPtr)
    {
        if (bulletPtr->bulletType != PlayerWhite && bulletPtr->bulletType != PlayerBlack)
        {
            bulletPtr++;
            continue;
        }
        struct Rectangle bulletRectangle = GetRectangle(bulletPtr->position, PLAYER_BULLET_COLLISION_HE);

        bool bulletHit = false;

        struct Enemy* enemyPtr = Enemies;
        struct Enemy* enemyEndPtr = Enemies + enemyCount;
        while (enemyPtr < enemyEndPtr)
        {
            struct Rectangle enemyRectangle = GetRectangle(enemyPtr->position, ENEMY_COLLISION_HE[enemyPtr->enemyType]);
            bulletHit = IntersectRect(bulletRectangle, enemyRectangle);
            if (bulletHit)
            {
                if ((enemyPtr->life & 3) == 0)
                {
                    SpawnRandomTinyExplosion(enemyPtr->position);
                }
                enemyPtr->life--;
                ScoreOnHit();
                break;
            }
            enemyPtr++;
        }

        if (bulletHit)
        {
            RemoveBullet(bulletPtr);
            bulletEndPtr--;
            break;
        }
        else
        {
            bulletPtr++;
        }
    }
}

void ShipDies()
{
    Ship.dieTransition = 120;
    FreezeFrame(4);
    SpawnPlayerExplosion(Ship.position);
}

bool IsSameColorAsShip(enum BulletType bulletType)
{
    return ( (bulletType == EnemySmallWhite && Ship.isWhite) ||
        (bulletType == EnemySmallBlack && (!Ship.isWhite)));
}

void TickBulletsDamagingShip()
{
    if (Ship.dieTransition || Ship.spawningTransition)
    {
        return;
    }
    struct Rectangle shipRectangle = GetRectangle(Ship.position, SHIP_COLLISION_HE);
    struct Rectangle shipRectangleForAcquiring = GetRectangle(Ship.position, SHIP_COLLISION_ACQUIRE_HE);

    struct Bullet* bulletPtr = bullets;
    struct Bullet* bulletEndPtr = bullets + bulletCount;
    while (bulletPtr < bulletEndPtr)
    {
        if ((bulletPtr->bulletType == PlayerWhite) || (bulletPtr->bulletType == PlayerBlack) || (bulletPtr->bulletType == PlayerWhiteTrail) || (bulletPtr->bulletType == PlayerBlackTrail))
        {
            bulletPtr++;
            continue;
        }
        bool hasHit;
        bool sameColor = IsSameColorAsShip(bulletPtr->bulletType);
        if (sameColor)
        {
            struct Rectangle bulletRectangle = GetRectangle(bulletPtr->position, ENEMY_BULLET_COLLISION_HE[0]);
            hasHit = IntersectRect(bulletRectangle, shipRectangleForAcquiring);
        }
        else
        {
            struct Rectangle bulletRectangle = GetRectangle(bulletPtr->position, ENEMY_BULLET_COLLISION_HE[0]);
            hasHit = IntersectRect(bulletRectangle, shipRectangle);
        }
        
        if (hasHit)
        {
            int jaugeAcquired = sameColor ? JAUGE_VALUE[0] : 0;
            bool die = !sameColor;

            Ship.jauge = min(Ship.jauge + jaugeAcquired, 255);
            if (jaugeAcquired)
            {
                jaugeBump = 4;
            }
            if (die)
            {
                ShipDies();
                return; // nothing more to do
            }
            RemoveBullet(bulletPtr);
            bulletEndPtr--;
            break;
        }
        else
        {
            bulletPtr++;
        }
    }
    // ship / enemy
    struct Enemy* enemyPtr = Enemies;
    struct Enemy* enemyEndPtr = Enemies + enemyCount;
    while (enemyPtr < enemyEndPtr)
    {
        struct Rectangle enemyRectangle = GetRectangle(enemyPtr->position, ENEMY_TO_SHIP_COLLISION_HE);
        if (IntersectRect(shipRectangle, enemyRectangle))
        {
            ShipDies();
            return;
        }
        enemyPtr++;
    }
}

void TickBullets()
{
    struct Bullet* bulletPtr = bullets;
    struct Bullet* bulletEndPtr = bullets + bulletCount;
    while (bulletPtr < bulletEndPtr)
    {
        bool removeBullet = false;
        if ((bulletPtr->bulletType == PlayerWhiteTrail) ||
            (bulletPtr->bulletType == PlayerBlackTrail))
        {
            struct Vector2 destination = (bulletPtr->enemyIndex == INVALID_ENEMY_INDEX) ? bulletPtr->destination : Enemies[bulletPtr->enemyIndex].position;
            struct Vector2 delta = V2Sub(destination, bulletPtr->position);
            bool hasHit = false;
            if (abs(delta.x.integer) < 20 && abs(delta.y.integer) < 20)
            {
                struct Fixed lengthSq = V2LengthSq(delta);
                hasHit = lengthSq.integer < 5;
            }
            
            if (hasHit)
            {
                removeBullet = true;
                // hit enemy
                if (bulletPtr->enemyIndex != INVALID_ENEMY_INDEX)
                {
                    struct Enemy* enemyPtr = &Enemies[bulletPtr->enemyIndex];
                    if ((enemyPtr->life & 3) == 0)
                    {
                        SpawnRandomTinyExplosion(enemyPtr->position);
                    }
                    enemyPtr->life -= 30;
                }
            }
            else
            {
                struct Vector2 targetDir = V2Normalize(delta);
                struct Fixed speed = FromInt(3 + min(bulletPtr->life, 7));
                struct Fixed interpolationStrength = Mul(FromFixed(0x0200), FromInt(min(bulletPtr->life, 20)));
                bulletPtr->direction = V2Lerp(bulletPtr->direction, targetDir, interpolationStrength);
                bulletPtr->position = V2Add(bulletPtr->position, V2Mul(bulletPtr->direction, speed));
                bulletPtr->trailPositions[bulletPtr->life & 7] = bulletPtr->position;
            }

            bulletPtr->life++;
            removeBullet |= (bulletPtr->life > 120);
        }
        else
        {
            bulletPtr->position = V2Add(bulletPtr->position, bulletPtr->direction);
            removeBullet = IsClipped(bulletPtr->position, -16);
        }

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