#include "Equilibrium.h"
#include "Infiltration.h"
#include <math.h>

/* M6 热平衡温度迭代求解模块
 * 问题：散热器供热能力 qsup 固定时，房间最终稳定在多少度？
 * 物理：qsup = Q_env(tn) + Q_leak(tn) = (S + C)·(tn - tw)
 *       其中 S = Σ(K'·A) 为围护总传热能力(W/K)，C 为渗透总系数(W/K)。
 *
 * 迭代（不动点形式）：
 *       tn_{k+1} = tw + (qsup - Q_leak(tn_k)) / S
 * 该式收敛到 tn = tw + qsup/(S+C)，即上述热平衡方程的唯一解。
 * Q_leak(tn_k) 复用 M5 的纯计算函数 leakLoss()，保证与设计工况同一口径。
 */
void equilibrium(void)
{
    double S = 0.0;
    for (int i = 0; i < MAX_FACE; i++)
        if (g_face[i].valid)
            S += g_face[i].kAdj * g_face[i].area;

    if (S <= 0.0) {
        printf("[错误] 围护总传热能力为 0，请先完成 M1(读围护)与 M3(系数修正)。\n");
        return;
    }

    double tnk = 18.0;          /* 初始猜测 = 常见设计室温 */
    double tnNew = tnk;
    int iter = 0;
    int converged = 0;

    printf("====== 热平衡迭代（Qsup = %.0f W） ======\n", g_wx.qsup);
    while (iter < 100) {
        tnNew = g_wx.tw + (g_wx.qsup - leakLoss(tnk)) / S;
        printf("第%d轮 tn=%.2lf  增量=%.2lf\n", iter + 1, tnNew, tnNew - tnk);
        iter++;
        if (fabs(tnNew - tnk) < 0.01) { converged = 1; break; }
        tnk = tnNew;
    }

    g_tn = tnNew;
    if (converged)
        printf("收敛。散热器 %.0f W 时平衡室温约 %.1f C\n", g_wx.qsup, g_tn);
    else
        printf("迭代超过 100 次仍未收敛，请检查供热能力。\n");
}
