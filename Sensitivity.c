#include "Sensitivity.h"
#include "HeatLoss.h"
#include "Infiltration.h"
#include "Equilibrium.h"

/* M8 室温敏感性实验模块
 * 表1：设定室温 14~24C，每 2C 一档，批量重算围护/渗透/总负荷；
 * 表2：散热器能力 2000~4000W，每 500W 一档，复用迭代求平衡室温。
 */
void sensitivity(void)
{
    /* 表 1：室温敏感性 */
    printf("\n====== 室温敏感性实验（14~24 C） ======\n");
    printf("室温C   围护热损失W   渗透热损失W   总负荷W\n");
    for (double t = 14.0; t <= 24.0 + 1e-9; t += 2.0) {
        double qEnv = heatLossFace(t);
        double qLeak = leakLoss(t);
        printf("%5.1f   %9.2f   %9.2f   %9.2f\n", t, qEnv, qLeak, qEnv + qLeak);
    }

    /* 表 2：供热能力敏感性 */
    printf("\n====== 供热能力敏感性实验（2000~4000 W） ======\n");
    printf("供热能力W   平衡室温C\n");
    for (double q = 2000.0; q <= 4000.0 + 1e-9; q += 500.0) {
        double teq = iterEquilibrium(q, 0);
        if (teq > -1e8)
            printf("%8.0f   %8.2f\n", q, teq);
        else
            printf("%8.0f   不收敛\n", q);
    }

    /* 恢复设计工况的全局状态（避免敏感实验留下的 t=24 值污染后续模块） */
    heatLossFace(tn);
    g_qLeak = leakLoss(tn);
    g_qTotal = g_qEnv + g_qLeak;
}
