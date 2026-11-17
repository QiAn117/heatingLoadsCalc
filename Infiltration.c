#include "Infiltration.h"

/* M5 冷风渗透热损失
 *
 * 缝隙法简化式：Q_渗 = 0.28 · c · ρ · L · (tn - tw)
 *   L  —— 缝隙渗风量 m3/h（= 每米缝隙渗风量 leak × 缝隙总长 gap）
 *   c  —— 1.0  kJ/(kg*C)
 *   ρ  —— 1.29 kg/m3
 *
 * 关于 0.28 和原文档里的 "/3.6"：
 *   Q = c·ρ·L·Δt 的量纲是 kJ/h，换成 W 需乘 1000/3600 ≈ 0.278（取 0.28）。
 *   即 0.28 里已经含了 kJ/h→W 的换算，若再除 3.6 就等于换算两次，
 *   与样例数值对不上（help.md 已指出这是文档笔误）。本模块按题面给的
 *   "0.28·c·ρ·leak·缝隙长·(tn-tw)" 实现，不再除 3.6。
 *   校核：0.28×1.0×1.29 = 0.3612 W/(m3/h·K)，
 *        与 1000/3600×1.005×1.29 ≈ 0.360 W/(m3/h·K) 基本一致。
 */

#define LEAK_COEF 0.28          /* 含 1000/3600 的单位换算系数 */

double leakFlowN(void)
{
    return g_wx.leak * g_wx.gapN;
}

double leakFlowS(void)
{
    return g_wx.leak * g_wx.gapS;
}

double leakCoeff(void)
{
    return LEAK_COEF * C_AIR * RHO_AIR * (leakFlowN() + leakFlowS());
}

double leakLoss(double tnRoom)
{
    return leakCoeff() * (tnRoom - g_wx.tw);
}

void infiltrationCalc(double tnRoom)
{
    double flowN = leakFlowN();
    double flowS = leakFlowS();
    double massN = RHO_AIR * flowN;             /* 质量流量 kg/h */
    double massS = RHO_AIR * flowS;
    double qN    = LEAK_COEF * C_AIR * RHO_AIR * flowN * (tnRoom - g_wx.tw);
    double qS    = LEAK_COEF * C_AIR * RHO_AIR * flowS * (tnRoom - g_wx.tw);

    g_qLeak = qN + qS;

    printf("\n====== 冷风渗透（tn = %.1f C，温差 %.1f C） ======\n",
           tnRoom, tnRoom - g_wx.tw);
    printf("北窗：缝隙 %.2f m  渗风量 %6.2f m3/h  质量流量 %6.2f kg/h  热损失 %7.2f W\n",
           g_wx.gapN, flowN, massN, qN);
    printf("南窗：缝隙 %.2f m  渗风量 %6.2f m3/h  质量流量 %6.2f kg/h  热损失 %7.2f W\n",
           g_wx.gapS, flowS, massS, qS);
    printf("渗透总系数 C = %.4f W/K   渗透热损失 g_qLeak = %.2f W\n",
           leakCoeff(), g_qLeak);
}
