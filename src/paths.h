struct Path
{
    struct Vector2* positions;
    unsigned short positionCount;
};

struct Path NewPath(unsigned positionCount)
{
    struct Path path;
    path.positions = (struct Vector2*)malloc(positionCount * sizeof(struct Vector2));
    path.positionCount = positionCount;
    return path;
}

struct Path ClonePath(struct Path* source)
{
    struct Path path;
    path = NewPath(source->positionCount);
    memcpy(path.positions, source->positions, source->positionCount * sizeof(struct Vector2));
    return path;
}

struct Path CloneSymetryPath(struct Path* source)
{
    struct Path res = ClonePath(source);
    for (int i = 0; i < res.positionCount; i++)
    {
        res.positions[i].y = Neg(res.positions[i].y);
    }
    return res;
}

void ReallocPath(struct Path* source, int newPositionCount)
{
    source->positions = (struct Vector2*)realloc(source->positions, newPositionCount * sizeof(struct Vector2));
    source->positionCount = newPositionCount;
}

struct Path Paths[512];