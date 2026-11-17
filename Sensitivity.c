#include "Sensitivity.h"
#include "HeatLoss.h"
#include "Infiltration.h"
#include "Equilibrium.h"
#include "SumLoad.h"

/* M8 室温/供热能力敏感性实验
 * 表1 固定围护参数，扫室温，看负荷随室温的变化（每档调用 M4、M5 的纯计算函数）；
 * 表2 固定房间，扫散热器能力，每档调用 M6 的迭代函数，体现迭代模块的复用。
 *
 * 两个表格函数只画"表体"，标题由调用者决定：
 * M8 给自己加 "======" 标题，M9 报表给它们加 "【六】【七】" 编号，
 * 这样屏幕与报表用的是同一份循环和同一批公式，不会出现两份数。
 *
 * 用整数计数循环生成温度/能力值：14,16,...,24 用 t += 2.0 恰好没问题，
 * 但一旦改成 0.1 步长就会因二进制误差漏掉最后一档，整数循环更稳。
 */

/* 返回 n = (end - begin)/step 的档数（四舍五入） */
static int stepCount(double begin, double end, double step)
{
    return (int)((end - begin) / step + 0.5);
}

void sensitivityTemps(FILE *fp)
{
    static const char *const head[] = { "围护热损失W", "渗透热损失W", "总负荷W" };
    int n = stepCount(SENS_TN_BEGIN, SENS_TN_END, SENS_TN_STEP);

    fprintPadded(fp, "室温C", 6);
    for (int i = 0; i < (int)(sizeof head / sizeof head[0]); i++) {
        fputs("  ", fp);
        if (i < (int)(sizeof head / sizeof head[0]) - 1) fprintPadded(fp, head[i], 12);
        else                                           fputs(head[i], fp);
    }
    fputc('\n', fp);

    for (int k = 0; k <= n; k++) {
        double t     = SENS_TN_BEGIN + k * SENS_TN_STEP;
        double qEnv  = heatLossFace(t);
        double qLeak = leakLoss(t);
        fprintf(fp, "%6.1f  %12.2f  %12.2f  %12.2f\n", t, qEnv, qLeak, qEnv + qLeak);
    }
}

void sensitivityCapacity(FILE *fp)
{
    int n = stepCount(SENS_Q_BEGIN, SENS_Q_END, SENS_Q_STEP);
    int bad = 0;                    /* 不收敛的档数 */

    fprintPadded(fp, "供热能力W", 8);
    fputs("  ", fp);
    fprintPadded(fp, "平衡室温C", 10);
    fputc('\n', fp);

    for (int k = 0; k <= n; k++) {
        double q = SENS_Q_BEGIN + k * SENS_Q_STEP;
        double teq = iterEquilibrium(q, 0);
        if (ITER_OK(teq)) {
            fprintf(fp, "%8.0f  %10.2f\n", q, teq);
        } else {
            fprintf(fp, "%8.0f  %10s\n", q, "不收敛");
            bad++;
        }
    }
    if (bad > 0)
        fprintf(fp, "[提示] %d 档供热能力未收敛，请检查围护面与散热器供热能力。\n", bad);
}

void sensitivity(void)
{
    printf("\n====== 室温敏感性实验（%.0f~%.0f C，步长 %.0f C） ======\n",
           SENS_TN_BEGIN, SENS_TN_END, SENS_TN_STEP);
    sensitivityTemps(stdout);

    printf("\n====== 供热能力敏感性实验（%.0f~%.0f W，步长 %.0f W） ======\n",
           SENS_Q_BEGIN, SENS_Q_END, SENS_Q_STEP);
    sensitivityCapacity(stdout);

    /* 复位：敏感性实验把 g_face[].q / g_qEnv 刷成了 24 C 的值，必须还原成设计工况，
     * 否则接下来直接输出报表会把 24 C 的热损失写进设计工况汇总。 */
    refreshDesignLoad();
    printf("\n[提示] 已恢复设计工况（tn = %.1f C）的计算结果：总负荷 %.2f W。\n",
           g_tnDesign, g_qTotal);
}
