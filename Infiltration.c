#include "Infiltration.h"
//Q=0.28·c·ρ·leak·缝隙长·(tn-tw)/3.6
double leakLoss(double tn)
{
    double Nleak = g_wx.leak * g_wx.gapN;
    double Sleak = g_wx.leak * g_wx.gapS;
    double qLeak = 0;
    double k_q = 0.28 * C_AIR * RHO * (tn - g_wx.tw);
    double qNleak = Nleak * k_q;
    double qSleak = Sleak * k_q;
    qLeak = qNleak + qSleak;
    printf("北窗渗风 %.2lf m3/h ", Nleak);
    printf("南窗渗风 %.2lf m3/h ", Sleak);
    printf("渗透热损失 %.2lf W\n", g_qLeak);
    return qLeak;
}