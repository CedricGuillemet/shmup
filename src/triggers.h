
struct Rectangle
{
    short left;
    short top;
    short right;
    short bottom;
};

bool IntersectRect(struct Rectangle r1, struct Rectangle r2)
{
    return !(r2.left > r1.right
        || r2.right < r1.left
        || r2.top > r1.bottom
        || r2.bottom < r1.top
        );
}

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
        struct Rectangle bulletRectangle;
        bulletRectangle.left = bulletPtr->position.x.integer - 16;
        bulletRectangle.right = bulletRectangle.left + 32;
        bulletRectangle.top = bulletPtr->position.y.integer - 16;
        bulletRectangle.bottom = bulletRectangle.top + 32;

        bool removeBullet = false;

        struct Enemy* enemyPtr = Enemies;
        struct Enemy* enemyEndPtr = Enemies + enemyCount;
        while (enemyPtr < enemyEndPtr)
        {
            struct Rectangle enemyRectangle;
            enemyRectangle.left = enemyPtr->position.x.integer - 8;
            enemyRectangle.right = enemyRectangle.left + 16;
            enemyRectangle.top = enemyPtr->position.y.integer - 8;
            enemyRectangle.bottom = enemyRectangle.top + 16;
            removeBullet = IntersectRect(bulletRectangle, enemyRectangle);
            if (removeBullet)
            {
                if ((enemyPtr->life & 3) == 0)
                {
                    SpawnRandomTinyExplosion(enemyPtr->position);
                }
                enemyPtr->life--;
                break;
            }
            enemyPtr++;
        }

        if (removeBullet)
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

void TickBulletsDamagingShip()
{
    if (Ship.dieTransition || Ship.spawningTransition)
    {
        return;
    }
    struct Rectangle shipRectangle;
    shipRectangle.left = Ship.position.x.integer - 8;
    shipRectangle.right = shipRectangle.left + 16;
    shipRectangle.top = Ship.position.y.integer - 8;
    shipRectangle.bottom = shipRectangle.top + 16;

    struct Bullet* bulletPtr = bullets;
    struct Bullet* bulletEndPtr = bullets + bulletCount;
    while (bulletPtr < bulletEndPtr)
    {
        if ((bulletPtr->bulletType == PlayerWhite) || (bulletPtr->bulletType == PlayerBlack) || (bulletPtr->bulletType == PlayerWhiteTrail) || (bulletPtr->bulletType == PlayerBlackTrail))
        {
            bulletPtr++;
            continue;
        }

        struct Rectangle bulletRectangle;
        bulletRectangle.left = bulletPtr->position.x.integer - 8;
        bulletRectangle.right = bulletRectangle.left + 16;
        bulletRectangle.top = bulletPtr->position.y.integer - 8;
        bulletRectangle.bottom = bulletRectangle.top + 16;

        bool removeBullet = IntersectRect(bulletRectangle, shipRectangle);
        if (removeBullet)
        {
            int jaugeAcquired = 0;
            bool die = false;
            switch (bulletPtr->bulletType)
            {
            case EnemySmallWhite:
                if (Ship.isWhite)
                {
                    jaugeAcquired = 8;
                }
                else
                {
                    die = true;
                }
                break;
            case EnemySmallBlack:
                if (!Ship.isWhite)
                {
                    jaugeAcquired = 8;
                }
                else
                {
                    die = true;
                }
                break;
            }
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
        struct Rectangle enemyRectangle;
        enemyRectangle.left = enemyPtr->position.x.integer - 8;
        enemyRectangle.right = enemyRectangle.left + 16;
        enemyRectangle.top = enemyPtr->position.y.integer - 8;
        enemyRectangle.bottom = enemyRectangle.top + 16;
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
            struct Vector2 destination = (bulletPtr->enemyIndex == 0xFFFF) ? bulletPtr->destination : Enemies[bulletPtr->enemyIndex].position;
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
                if (bulletPtr->enemyIndex != 0xFFFF)
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