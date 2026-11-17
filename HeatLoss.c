#include "HeatLoss.h"

/* M4 围护结构传热热损失
 * 公式：q_i = K'·A·(tn - tw)，g_qEnv = Σq_i      （题目公式 Q_传 = K'·A·(tn-tw)）
 * 说明：热损失只随室温线性变化，所以把纯计算（heatLossFace）与打印（heatlossCalc）分开，
 *       M6/M7/M8/M9 反复调用纯函数时不会刷屏。
 */

double envCoeff(void)
{
    double s = 0.0;

    for (int i = 0; i < g_faceCnt; i++) {
        if (!g_face[i].valid) continue;
        s += g_face[i].kAdj * g_face[i].area;
    }
    return s;
}

double heatLossFace(double tnRoom)
{
    g_qEnv = 0.0;
    for (int i = 0; i < g_faceCnt; i++) {
        if (!g_face[i].valid) continue;
        g_face[i].q = g_face[i].kAdj * g_face[i].area * (tnRoom - g_wx.tw);
        g_qEnv += g_face[i].q;
    }
    return g_qEnv;
}

void heatlossCalc(double tnRoom)
{
    static const char *const head[] = { "朝向", "面积m2", "K修正", "温差C", "热损失W", "占比" };
    int    validCnt = 0;
    double dt = tnRoom - g_wx.tw;

    heatLossFace(tnRoom);

    printf("\n====== 围护结构传热热损失（tn = %.1f C，tw = %.1f C，温差 %.1f C） ======\n",
           tnRoom, g_wx.tw, dt);
    printPadded("名称", 10);
    printHeader(head, (int)(sizeof head / sizeof head[0]));

    for (int i = 0; i < g_faceCnt; i++) {
        double pct;
        if (!g_face[i].valid) continue;

        pct = (g_qEnv > 0.0) ? g_face[i].q / g_qEnv * 100.0 : 0.0;
        validCnt++;
        printPadded(g_face[i].name, 10);
        printf("  %-6c  %6.1f  %6.2f  %6.1f  %6.1f  %5.1f%%\n",
               g_face[i].orient, g_face[i].area, g_face[i].kAdj,
               dt, g_face[i].q, pct);
    }
    if (validCnt == 0)
        printf("（无有效围护面）\n");

    printf("围护传热热损失 g_qEnv = %.2f W\n", g_qEnv);
}
