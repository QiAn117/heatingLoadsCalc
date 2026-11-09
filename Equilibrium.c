#include "Equilibrium.h"
void equilibrium()
{
    double tnkp1 = 0;
    double tnk = tn;
    double s = 0;
    int i;
    for(i = 0; i < MAX_FACE; i++)
    {
        if(g_face[i].valid)
        {
            s += g_face[i].kAdj * g_face[i].area;// + C_AIR * g_wx.leak * g_wx.gapN;
        }
    }
    //tn_{k+1} = tw + (Q_sup − C·(tn_k − tw)) / S
    i = 1;
    do{
        tnkp1 = g_wx.tw + (g_wx.qsup - C_AIR * (tnk - g_wx.tw)) / s;
        printf("第%d轮 tn=%.2lf 增量%.2lf\n", i, tnkp1, tnkp1 - tnk);
        tnk = tnkp1;
        i++;
        if (i > 100)
        {
            printf("不收敛 请检查供热能力");
            break;
        }
    }while(fabs(tnkp1 - tnk) >= 0.01);
}