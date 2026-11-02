#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "KCorrect.h"
#include "HeatLoss.h"
struct Envelope g_face[MAX_FACE];
struct Weather  g_wx;
int    g_faceCnt;        /* 围护面数 */
double tn;
double g_tn;             /* 设定/平衡室温 C，迭代求出 */
double g_qEnv, g_qLeak, g_qTotal; /* 围护热损失/渗透热损失/总负荷 W */
void givenDebugVal();
int main()
{
    system("chcp 65001 > NUL");
    givenDebugVal();
    kCorrect();
    heatlossCalc();
    return 0;
}

void givenDebugVal()
{
    int i = 0;
    do
    {
        //g_face[0].name = "北外墙";
        strcpy(g_face[i].name, "北外墙");
        g_face[i].orient = 'N';
        g_face[i].area = 10.0;
        g_face[i].k = 0.45;
        g_face[i].valid = 1;//test,it factly need to be set by code, after check the legality of vals
        i++;
    } while (i < 2);
    do
    {
        //g_face[0].name = "北外墙";
        g_wx.tw = -19;
        g_wx.alphaN = 1.10;
        g_wx.alphaS = 1.10;
        g_wx.alphaEW = 1.00;
    } while (0);
    tn = 18.0;

}