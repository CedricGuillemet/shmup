
int jaugeBump = 0;
int freezeFrame = 0;

void DrawHUD(unsigned char jauge)
{
    //Ship.jauge = 255;
    int halfJaugeBump = jaugeBump >> 1;
    int jaugeValue = jauge >> 3;
    DrawRectangle(V2FromInt(6 + halfJaugeBump/2, 35 + jaugeBump/2), V2FromInt(4 + halfJaugeBump, 32 + jaugeBump), 0);
    DrawRectangle(V2FromInt(6 + halfJaugeBump/2, 4 + jaugeValue + jaugeBump/2), V2FromInt(3 + halfJaugeBump, jaugeValue + jaugeBump), 15);

    DrawNumber(50, 2, score, MAX_DENUM);
    for (int i = 0; i < combo; i ++)
    {
        DrawRectangle(V2FromInt(140 + i * 10, 5), V2FromInt(4, 4), (comboType == ComboWhite) ? 15 : 0);
    }

    DrawText(170, 2, "X");
    if (comboMultiplier > 999)
    {
        DrawNumber(180, 2, comboMultiplier, 1000);
    }
    else if (comboMultiplier > 99)
    {
        DrawNumber(180, 2, comboMultiplier, 100);
    }
    else if (comboMultiplier > 9)
    {
        DrawNumber(180, 2, comboMultiplier, 10);
    }
    else
    {
        DrawNumber(180, 2, comboMultiplier, 1);
    }
}

void TickHUD(unsigned char jauge, int globalFrame)
{
    if (jauge == 255)
    {
        jaugeBump = globalFrame & 0xF;
        if (jaugeBump < 8)
        {
            jaugeBump >>= 2;
        }
        else
        {
            jaugeBump = 4 - ((jaugeBump - 8) >> 1);
        }
    }
    else
    {
        if (jaugeBump > 0)
        {
            jaugeBump--;
        }
    }
    if (freezeFrame > 0)
    {
        freezeFrame --;
    }
}

void FreezeFrame(int frameCount)
{
    freezeFrame = frameCount;
}