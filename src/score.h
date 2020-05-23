
unsigned int score = 0;
unsigned int combo = 0;
unsigned int comboMultiplier = 0;

enum ComboType comboType = ComboUndefined;

void ScoreOnHit()
{
    score += HIT_SCORE;
}

void ScoreOnDestroy(enum EnemyType enemyType)
{
    score += DESTROY_SCORE[enemyType] * (comboMultiplier + 1);
    if (comboType == ComboUndefined)
    {
        combo = 1;
    }
    else
    {
        if (comboType != DESTROY_COMBO[enemyType])
        {
            combo = 1;
            comboMultiplier = 0;
        }
        else
        {
            combo ++;
            if (combo == 3)
            {
                // combo accomplished
                comboType = ComboUndefined;
                comboMultiplier ++;
                combo = 0;
                return;
            }
        }
    }
    
    comboType = DESTROY_COMBO[enemyType];
}