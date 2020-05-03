struct Path
{
    struct Vector2* positions;
    unsigned short positionCount;
};

struct Path NewPath(unsigned positionCount)
{
    struct Path path;
    path.positions = (struct Vector2*)malloc(positionCount * sizeof(struct Vector2));
    return path;
}

struct Path Paths[1];