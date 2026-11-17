#include "Equilibrium.h"
#include "HeatLoss.h"
#include "Infiltration.h"
#include <math.h>

/* M6 热平衡温度迭代求解
 *
 * 平衡工况的物理含义：散热器供多少热，房间就散多少热
 *      qsup = Q_env(tn) + Q_leak(tn) = (S + C)·(tn - tw)
 * 其中 S = Σ(K'·A) 为围护总传热能力（W/K），C 为渗透总系数（W/K）。
 *
 * 不动点迭代（题面公式，把渗透项留在右边）：
 *      tn(k+1) = tw + [ qsup - Q_leak(tn(k)) ] / S
 * 误差按 C/S 的比率收缩，所以 C < S 时收敛得很快（本数据约 4 轮）。
 * 但 C ≥ S（房间围护很小、缝隙很大）时该映射不再是压缩映射，会发散，
 * 原实现会打印 100 轮天文数字后报"不收敛"，属于误导性结论。
 * 处理办法：C ≥ S 时改用等价且无条件收敛的残差修正式
 *      tn(k+1) = tn(k) + [ qsup - (Q_env + Q_leak) ] / (S + C)
 * 它在同一点收敛，只是每步用总系数做分母。
 */

/* 打印/写出"供热能力是否足够"的结论，M6 屏幕与 M9 报表共用这段判断 */
void printCapacityNote(FILE *fp, double qsup, double teq)
{
    double need = (envCoeff() + leakCoeff()) * (g_tnDesign - g_wx.tw);

    if (teq < g_tnDesign - 0.5)
        fprintf(fp, "→ 平衡室温比设计室温 %.1f C 低 %.1f C，供热能力不足，"
                    "需加大散热面积或提高供水温度。\n", g_tnDesign, g_tnDesign - teq);
    else if (teq > g_tnDesign + 0.5)
        fprintf(fp, "→ 平衡室温比设计室温 %.1f C 高 %.1f C，供热能力有富余。\n",
                g_tnDesign, teq - g_tnDesign);
    else
        fprintf(fp, "→ 平衡室温与设计室温基本一致，供热能力匹配。\n");

    fprintf(fp, "   达到设计室温 %.1f C 所需供热能力约 %.0f W，当前散热器 %.0f W。\n",
            g_tnDesign, need, qsup);
}

double iterEquilibrium(double qsup, int verbose)
{
    double S = envCoeff();
    double C = leakCoeff();
    double tnk, tnNew;
    int    stable, converged = 0;

    g_iterCnt = 0;

    if (S <= 0.0) {
        if (verbose)
            printf("[错误] 围护总传热能力 S = 0，请先完成 M1(读围护)与 M3(系数修正)。\n");
        return ITER_FAIL;
    }
    if (qsup <= 0.0) {
        if (verbose)
            printf("[错误] 散热器供热能力 %.0f W 无效（应大于 0），请检查 %s。\n",
                   qsup, WEATHER_FILE);
        return ITER_FAIL;
    }

    stable = (C < S);

    if (verbose) {
        printf("\n====== 热平衡迭代（Qsup = %.0f W，S = %.2f W/K，C = %.4f W/K） ======\n",
               qsup, S, C);
        if (!stable)
            printf("[提示] 渗透系数 C 不小于围护系数 S，不动点映射不收敛，\n"
                   "       改用残差修正式 tn += (qsup - 总损失)/(S+C)。\n");
    }

    tnk  = g_tnDesign;              /* 初始猜测：取设计室温 */
    tnNew = tnk;

    while (g_iterCnt < ITER_MAX) {
        double delta;

        if (stable)
            tnNew = g_wx.tw + (qsup - leakLoss(tnk)) / S;
        else
            tnNew = tnk + (qsup - (S * (tnk - g_wx.tw) + leakLoss(tnk))) / (S + C);

        g_iterCnt++;
        delta = tnNew - tnk;

        if (!isfinite(tnNew)) {     /* 数值溢出兜底，避免打印 inf */
            if (verbose) printf("第%2d轮 数值溢出，迭代中止。\n", g_iterCnt);
            break;
        }
        converged = (fabs(delta) < ITER_EPS);

        if (verbose)
            printf("第%2d轮  tn = %8.3f C   增量 = %+8.3f C%s\n",
                   g_iterCnt, tnNew, delta, converged ? "   ← 收敛" : "");
        if (converged) break;

        tnk = tnNew;
    }

    if (!converged) {
        if (verbose)
            printf("[错误] 迭代 %d 轮仍未收敛，请检查 %s 中的参数是否合理。\n",
                   ITER_MAX, WEATHER_FILE);
        return ITER_FAIL;
    }

    if (verbose) {
        printf("收敛：散热器 %.0f W 时平衡室温约 %.2f C（共 %d 轮）。\n",
               qsup, tnNew, g_iterCnt);
        printCapacityNote(stdout, qsup, tnNew);
    }
    return tnNew;
}

void equilibrium(void)
{
    double teq = iterEquilibrium(g_wx.qsup, 1);

    if (!ITER_OK(teq)) return;
    g_tn = teq;
}
