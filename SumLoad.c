#include "SumLoad.h"
#include "HeatLoss.h"
#include "Infiltration.h"

/* M7 采暖负荷汇总模块
 * 设计工况：室温 tn 固定为设计值，Q_total = Q_env + Q_leak。
 * 复用 M4(heatlossCalc) 与 M5(leakLoss)，再按房间面积算单位面积指标，
 * 并与 50~80 W/m2 经验范围对比。
 */
double sumLoad(void)
{
    printf("请输入设计室温 tn (C)：");
    if (scanf("%lf", &tn) != 1) { tn = 18.0; while (getchar() != '\n'); }

    heatlossCalc();                 /* 逐面热损失 + g_qEnv */
    g_qLeak = leakLoss(tn);         /* 渗透热损失（复用 M5 纯函数） */

    double nLeak = g_wx.leak * g_wx.gapN;
    double sLeak = g_wx.leak * g_wx.gapS;
    printf("\n====== 冷风渗透 ======\n");
    printf("北窗渗风 %.2f m3/h  南窗渗风 %.2f m3/h  渗透热损失 %.2f W\n",
           nLeak, sLeak, g_qLeak);

    g_qTotal = g_qEnv + g_qLeak;

    /* 房间面积：优先取地面(U)面积，缺省取屋面(R)，再缺省取各面面积和 */
    double roomArea = 0.0;
    for (int i = 0; i < MAX_FACE; i++)
        if (g_face[i].valid && g_face[i].orient == 'U') roomArea = g_face[i].area;
    if (roomArea <= 0.0)
        for (int i = 0; i < MAX_FACE; i++)
            if (g_face[i].valid && g_face[i].orient == 'R') roomArea = g_face[i].area;
    if (roomArea <= 0.0)
        for (int i = 0; i < MAX_FACE; i++)
            if (g_face[i].valid) roomArea += g_face[i].area;

    double indicator = (roomArea > 0.0) ? g_qTotal / roomArea : 0.0;

    printf("\n====== 设计工况汇总 ======\n");
    printf("围护传热热损失 %.2f W   渗透热损失 %.2f W\n", g_qEnv, g_qLeak);
    printf("总负荷 %.2f W   房间面积 %.2f m2   单位面积指标 %.2f W/m2\n",
           g_qTotal, roomArea, indicator);
    if (indicator >= 50.0 && indicator <= 80.0)
        printf("（经验范围 50~80 W/m2，合理）\n");
    else if (indicator < 50.0)
        printf("（经验范围 50~80 W/m2，偏低，请核查围护参数与房间面积）\n");
    else
        printf("（经验范围 50~80 W/m2，偏高，请核查围护参数与房间面积）\n");

    return g_qTotal;
}
