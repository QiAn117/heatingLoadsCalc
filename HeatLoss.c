#include "HeatLoss.h"
#include <stdio.h>

/* M4 围护热损失逐面累加模块
 * 对每个有效围护面按 q = kAdj·area·(tn-tw) 计算热损失并累加得 g_qEnv，
 * 打印逐面热损失表与占比。
 */
void heatlossCalc()
{
    g_qEnv = 0.0;
    for (int i = 0; i < MAX_FACE; i++) {
        if (g_face[i].valid) {
            g_face[i].q = g_face[i].kAdj * g_face[i].area * (tn - g_wx.tw);
            g_qEnv += g_face[i].q;
        }
    }

    printf("名称    朝向  面积m2   K修正   热损失W   占比\n");
    for (int i = 0; i < MAX_FACE; i++) {
        if (g_face[i].valid) {
            double pct = (g_qEnv > 0.0) ? g_face[i].q / g_qEnv * 100.0 : 0.0;
            printf("%-8s  %c   %6.1f   %5.2f   %8.1f   %5.1f%%\n",
                   g_face[i].name, g_face[i].orient, g_face[i].area,
                   g_face[i].kAdj, g_face[i].q, pct);
        }
    }
    printf("围护传热热损失 g_qEnv = %.2f W\n", g_qEnv);
}
