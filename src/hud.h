
int jaugeBump = 0;
int freezeFrame = 0;

void DrawHUD(unsigned char jauge)
{
    //Ship.jauge = 255;
    int halfJaugeBump = jaugeBump >> 1;
    int jaugeValue = jauge >> 3;
    DrawRectangle(V2FromInt(35 + jaugeBump / 2, 14 + halfJaugeBump/2), V2FromInt(32 + jaugeBump, 4 + halfJaugeBump), 0);
    DrawRectangle(V2FromInt(4 + jaugeValue + jaugeBump / 2, 14 + halfJaugeBump/2), V2FromInt(jaugeValue + jaugeBump, 3 + halfJaugeBump), 15);

    DrawNumber(2, 2, score, MAX_DENUM);
    for (unsigned int i = 0; i < combo; i ++)
    {
        DrawRectangle(V2FromInt(98 + i * 10, 5), V2FromInt(4, 4), (comboType == ComboWhite) ? 15 : 0);
    }

    DrawText(122, 2, "X");
    if (comboMultiplier > 999)
    {
        DrawNumber(132, 2, comboMultiplier, 1000);
    }
    else if (comboMultiplier > 99)
    {
        DrawNumber(132, 2, comboMultiplier, 100);
    }
    else if (comboMultiplier > 9)
    {
        DrawNumber(132, 2, comboMultiplier, 10);
    }
    else
    {
        DrawNumber(132, 2, comboMultiplier, 1);
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