#include "Equilibrium.h"
#include "Infiltration.h"
#include <math.h>

/* 不动点迭代求解平衡室温：
 *   tn_{k+1} = tw + (qsup - Q_leak(tn_k)) / S
 * 其中 S = Σ(K'·A) 为围护总传热能力(W/K)，Q_leak 复用 M5 纯函数 leakLoss()。
 * 该式收敛到 tn = tw + qsup/(S+C)，即 qsup = (S+C)·(tn-tw) 的唯一解。
 * verbose=1 时打印每轮过程；不收敛返回 -1e9。
 */
double iterEquilibrium(double qsup, int verbose)
{
    double S = 0.0;
    for (int i = 0; i < MAX_FACE; i++)
        if (g_face[i].valid)
            S += g_face[i].kAdj * g_face[i].area;

    if (S <= 0.0) {
        if (verbose)
            printf("[错误] 围护总传热能力为 0，请先完成 M1(读围护)与 M3(系数修正)。\n");
        return -1e9;
    }

    if (verbose)
        printf("====== 热平衡迭代（Qsup = %.0f W） ======\n", qsup);

    double tnk = 18.0;          /* 初始猜测 = 常见设计室温 */
    double tnNew = tnk;
    int iter = 0;
    int converged = 0;

    while (iter < 100) {
        tnNew = g_wx.tw + (qsup - leakLoss(tnk)) / S;
        if (verbose)
            printf("第%d轮 tn=%.2lf  增量=%.2lf\n", iter + 1, tnNew, tnNew - tnk);
        iter++;
        if (fabs(tnNew - tnk) < 0.01) { converged = 1; break; }
        tnk = tnNew;
    }

    if (verbose) {
        if (converged)
            printf("收敛。散热器 %.0f W 时平衡室温约 %.1f C\n", qsup, tnNew);
        else
            printf("迭代超过 100 次仍未收敛，请检查供热能力。\n");
    }
    return converged ? tnNew : -1e9;
}

/* M6 菜单入口 */
void equilibrium(void)
{
    double t = iterEquilibrium(g_wx.qsup, 1);
    if (t > -1e8) g_tn = t;
}
