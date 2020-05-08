struct Ship_t
{
    struct Vector2 position;
    bool isWhite;
    bool switchColorReleased;
    
    unsigned char switchTransition;
    unsigned char spawningTransition;
    unsigned char jauge;
    unsigned char dieTransition;
};

struct Ship_t Ship;

void SpawnShip()
{
    V2SetInt(&Ship.position, -16, 100);
    Ship.isWhite = true;
    Ship.switchColorReleased = true;
    Ship.switchTransition = 0;
    Ship.spawningTransition = 60;
    Ship.jauge = 255;
    Ship.dieTransition = 0;
}

void MoveShip(struct Vector2 direction)
{
    Ship.position = V2Add(Ship.position, V2Mul(direction, FromFixed(0x18000)));

    int margin = 14;
    if (Ship.position.x.integer < margin)
    {
        SetInt(&Ship.position.x, margin);
    }

    if (Ship.position.x.integer > SCREEN_WIDTH - margin)
    {
        SetInt(&Ship.position.x, SCREEN_WIDTH - margin);
    }

    if (Ship.position.y.integer < margin)
    {
        SetInt(&Ship.position.y, margin);
    }

    if (Ship.position.y.integer > SCREEN_HEIGHT - margin)
    {
        SetInt(&Ship.position.y, SCREEN_HEIGHT - margin);
    }
}

void TickShip(bool left, bool right, bool up, bool down, bool fire, bool switchColor, bool discharge)
{
    struct Vector2 direction;
    V2SetInt(&direction, 0, 0);
    struct Vector2 directionUp, directionDown, directionLeft, directionRight;
    V2SetInt(&directionUp, 0, -1);
    V2SetInt(&directionDown, 0, 1);
    V2SetInt(&directionLeft, -1, 0);
    V2SetInt(&directionRight, 1, 0);

    if (Ship.dieTransition > 0)
    {
        Ship.dieTransition--;
        if (!Ship.dieTransition)
        {
            SpawnShip();
        }
        return;
    }

    if (left)
    {
        direction = V2Add(direction, directionLeft);
    }
    if (right)
    {
        direction = V2Add(direction, directionRight);
    }
    if (up)
    {
        direction = V2Add(direction, directionUp);
    }
    if (down)
    {
        direction = V2Add(direction, directionDown);
    }
    if (Ship.spawningTransition > 0)
    {
        Ship.spawningTransition--;
        if (Ship.spawningTransition > 30)
        {
            V2SetFixed(&direction, FromFixed(0x8000), FromFixed(0));
        }
    }

    if (!Ship.spawningTransition)
    {
        if (!Ship.switchTransition)
        {
            if (fire)
            {
                struct Vector2 directionBullet;
                V2SetInt(&directionBullet, 20, 0);
                SpawnBullet(Ship.position, directionBullet, Ship.isWhite ? PlayerWhite : PlayerBlack);
            }
            if (discharge && Ship.jauge > 0)
            {
                int trailCount = max(Ship.jauge >> 4, 1);
                if (trailCount == 1)
                {
                    unsigned short destination = enemyCount ? 0 : INVALID_ENEMY_INDEX;
                    SpawnTrail(Ship.position, V2FromInt(-1, 0), Ship.isWhite ? PlayerWhiteTrail : PlayerBlackTrail, destination);
                }
                else
                {
                    for (int i = 0; i < trailCount; i++)
                    {
                        struct Fixed step = Div(FromInt(1), FromInt(trailCount - 1));
                        struct Fixed t = Mul(step, FromInt(i));
                        struct Vector2 directionBullet = V2Normalize(V2Lerp(V2FromInt(-1, 1), V2FromInt(-1, -1), t));
                        unsigned short destination = enemyCount ? (i % enemyCount) : INVALID_ENEMY_INDEX;
                        SpawnTrail(Ship.position, directionBullet, Ship.isWhite ? PlayerWhiteTrail : PlayerBlackTrail, destination);
                    }
                }
                Ship.jauge = 0;
            }
        }
        if (Ship.switchColorReleased && switchColor)
        {
            Ship.isWhite = !Ship.isWhite;
            Ship.switchTransition = 20;
        }
    }
    Ship.switchColorReleased = !switchColor;
    if (Ship.switchTransition > 0)
    {
        Ship.switchTransition--;
    }

    MoveShip(direction);
}

void DrawShip()
{
    if (Ship.dieTransition)
    {
        return;
    }
    struct Vector2 halfExtend;
    V2SetInt(&halfExtend, 16, 16);
    if (!(Ship.spawningTransition&1))
    {
        DrawRectangle(Ship.position, halfExtend, Ship.isWhite?11:5);
    }
}
