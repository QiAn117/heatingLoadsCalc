#include "HeatLoss.h"
#include <stdio.h>
void heatlossCalc()
{
    printf("M4 start to run\n");//debug line
    //printf("名称    朝向  面积m2   K原始   alpha   K修正   热损失W   占比");
    g_qEnv = 0.0;
    for(int i = 0; i < MAX_FACE; i++)
    {
        if(g_face[i].valid == 1)
        {
            g_face[i].q = g_face[i].kAdj * g_face[i].area * (tn - g_wx.tw);
            g_qEnv += g_face[i].q;
            //printf("q%d = %.2lf\n", i, g_face[i].q);//debug line
            printf("%-8s q = %.2lf\n", g_face[i].name, g_face[i].q);
        }
        //printf("%-8s %c   %.2f   %.2f   %.2f   %.2f\n", g_face[i].name, g_face[i].orient, g_face[i].area, g_face[i].k, g_face[i].kAdj, g_face[i].kAdj);

    }
    printf("g_qEnv = %.2lf\n",g_qEnv);
}