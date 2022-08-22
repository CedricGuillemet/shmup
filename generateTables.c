
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main()
{
    FILE *fp = fopen("src/tables.h", "wt");
    if (fp)
    {
        fputs("static const struct Fixed cosines[2048] = {\n", fp);
        for (int i = 0;i<2048;i++)
        {
            float angle = (i / 2048.f) * 3.141592f * 2.f;
            float cs = cosf(angle) * 65536.f;
            if (cs >= 0.f)
            {
                fprintf(fp, "0x%x,\n", (int)(cs));
            }
            else
            {
                fprintf(fp, "-0x%x,\n", (int)(-cs));
            }
        }
        
        fputs("};\nstruct Fixed tans[1024] = {\n", fp);
        for (int i = 0;i<1024;i++)
        {
            float angle = (i / 1024.f) * 3.141592f;
            float t = tanf(angle) * 65536.f;
            if (t >= 0.f)
            {
                fprintf(fp, "0x%x,\n", (int)(t));
            }
            else
            {
                fprintf(fp, "-0x%x,\n", (int)(-t));
            }
        }
        fputs("};\n", fp);
        fclose(fp);
        return 0;
    }
    return 1;
}