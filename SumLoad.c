#include "SumLoad.h"
#include "HeatLoss.h"
#include "Infiltration.h"

/* M7 采暖负荷汇总模块
 * 设计工况：室温固定为设计值 g_tnDesign，Q_total = Q_env + Q_leak。
 * 复用 M4(heatlossCalc) 与 M5(infiltrationCalc)，按房间面积算单位面积指标，
 * 并与 50~80 W/m2 经验范围对比。
 */

/* 房间面积：地面面积最能代表"房间面积"，其次屋面；两者都没有时退化为各面之和。
 * 注意不能直接累加所有围护面——那样得到的是"展开面积"，指标会偏小。 */
double roomArea(void)
{
    double area = 0.0;
    int    hasU = 0;
    int    hasR = 0;

    for (int i = 0; i < g_faceCnt; i++) {
        if (!g_face[i].valid) continue;
        if (g_face[i].orient == 'U') { area = g_face[i].area; hasU = 1; }
    }
    if (!hasU) {
        for (int i = 0; i < g_faceCnt; i++) {
            if (!g_face[i].valid) continue;
            if (g_face[i].orient == 'R') { area = g_face[i].area; hasR = 1; }
        }
    }
    if (!hasU && !hasR) {
        area = 0.0;
        for (int i = 0; i < g_faceCnt; i++) {
            if (!g_face[i].valid) continue;
            area += g_face[i].area;
        }
    }
    return area;
}

double loadIndicator(double qTotal)
{
    double area = roomArea();

    return (area > 0.0) ? qTotal / area : 0.0;
}

void refreshDesignLoad(void)
{
    heatLossFace(g_tnDesign);
    g_qLeak  = leakLoss(g_tnDesign);
    g_qTotal = g_qEnv + g_qLeak;
}

double sumLoad(void)
{
    double area;
    double indicator;

    heatlossCalc(g_tnDesign);       /* M4：逐面热损失 + g_qEnv */
    infiltrationCalc(g_tnDesign);   /* M5：过程量 + g_qLeak */

    area      = roomArea();
    g_qTotal  = g_qEnv + g_qLeak;
    indicator = loadIndicator(g_qTotal);

    printf("\n====== 设计工况汇总 ======\n");
    printf("围护传热热损失 %.2f W   渗透热损失 %.2f W\n", g_qEnv, g_qLeak);
    printf("总负荷 %.2f W   房间面积 %.2f m2   单位面积指标 %.2f W/m2\n",
           g_qTotal, area, indicator);

    if (area <= 0.0) {
        printf("[警告] 没有有效围护面，房间面积与负荷指标无法计算。\n");
    } else if (indicator >= IDX_LO && indicator <= IDX_HI) {
        printf("（经验范围 %.0f~%.0f W/m2，合理）\n", IDX_LO, IDX_HI);
    } else if (indicator < IDX_LO) {
        printf("（经验范围 %.0f~%.0f W/m2，偏低，请核查围护参数与房间面积）\n",
               IDX_LO, IDX_HI);
    } else {
        printf("（经验范围 %.0f~%.0f W/m2，偏高，请核查围护参数与房间面积）\n",
               IDX_LO, IDX_HI);
    }

    return g_qTotal;
}
