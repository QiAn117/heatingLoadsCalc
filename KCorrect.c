#include "KCorrect.h"
void kCorrect()
{

    for(int i = 0; i < MAX_FACE; i++)
    {
        if(g_face[i].valid == 1)
        {
            double adj = 0.0;
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
                adj = 1.00;
                break;      
            case 'W':
            case 'E':
                adj = g_wx.alphaEW;
                break;  
            default:
                break;
            }
            g_face[i].kAdj = g_face[i].k * adj;
        }
    }
}
