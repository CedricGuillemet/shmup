struct Input_t
{
    bool left;
    bool right;
    bool up;
    bool down;
    bool fire;
    bool switchColor;
    bool discharge;
};

int warping = 0;

void GameWarping()
{
    DrawSpeed();

    static int flick = 0;

    warping++;

    if (warping < 150)
    {
        fovy.value = 60622 - warping * 200;
        Ship.position = V2Lerp(Ship.position, V2FromInt(220, 100), FromFixed(0x600));
        Ship.switchTransition = (150 - warping) / 2;
        DrawShip(0);
    }
    else if (warping < 300)
    {
        fovy.value += 200;
        Ship.position = V2Sub(Ship.position, V2FromInt(1, 0));
        DrawShip(((++flick) & 1) * ((warping - 150) / 10));
    }
    else if (warping < 400)
    {
        Ship.position = V2Lerp(Ship.position, V2FromInt(300, 100), FromFixed(0x1000));
        DrawShip(((++flick) & 3) * 4);
        paletteFade = ((warping - 300) / 2) * 6;
    }
    else
    {
        SpawnShip();
        FadePalette(0);
        warping = 0;
    }
}

void GamePlay(struct Input_t Input)
{
    if (paletteFade > 0)
        paletteFade--;
    else if (paletteFade < 0)
        paletteFade++;

    TickHUD(Ship.jauge, GlobalFrame);
    if (freezeFrame == 0)
    {
        TickShip(Input.left, Input.right, Input.up, Input.down, Input.fire, Input.switchColor, Input.discharge);
        TickOrchestra();
        TickBullets();
        TickEnemies();
        TickEffects();


        TickBulletsDamagingEnemies();
        TickBulletsDamagingShip();

        DrawLevel();
        DrawBullets(0);

        //DrawSprite(V2FromInt(100,100), remappedShoot, 32, 64, false);

        DrawEnemies();
        DrawBullets(1);
        DrawShip(0);
        DrawEffects();
        DrawHUD(Ship.jauge);
    }
}

void GameLoop(struct Input_t Input)
{
    ComputeMatrices();
    FadePalette(paletteFade);
    if (warping)
    {
        GameWarping();
    }
    else
    {
        GamePlay(Input);
    }
}