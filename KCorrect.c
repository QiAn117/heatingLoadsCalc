#include "KCorrect.h"
void kCorrect()
{
    printf("M3 start to run\n");//debug line
    for(int i = 0; i < MAX_FACE; i++)
    {
        if(g_face[i].valid == 1)
        {
            double adj = 1.0;
            switch (g_face[i].orient)
            {
            case 'N':
                adj = g_wx.alphaN;
                break;
            case 'S':
                adj = g_wx.alphaS;
                break;
            case 'R':
            case 'U':
                break;      
            case 'W':
            case 'E':
                adj = g_wx.alphaEW;
                break;  
            default:
                break;
            }
            g_face[i].kAdj = g_face[i].k * adj;
            printf("%-8s kAdj = %.2lf\n", g_face[i].name, g_face[i].kAdj);
        }
    }
}
