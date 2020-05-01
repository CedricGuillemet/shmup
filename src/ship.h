struct Ship_t
{
    struct Vector2 position;
};

struct Ship_t Ship;

void MoveShip(struct Vector2 direction)
{
    Ship.position = V2Add(Ship.position, direction);
}

void TickShip(bool left, bool right, bool up, bool down)
{
    struct Vector2 direction;
    V2SetInt(&direction, 0, 0);
    struct Vector2 directionUp, directionDown, directionLeft, directionRight;
    V2SetInt(&directionUp, 0, -1);
    V2SetInt(&directionDown, 0, 1);
    V2SetInt(&directionLeft, -1, 0);
    V2SetInt(&directionRight, 1, 0);

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

    MoveShip(direction);
}