#include "Report.h"
#include "HeatLoss.h"
#include "Infiltration.h"
#include "Equilibrium.h"

/* M9 负荷报表输出模块
 * 把参数清单、逐面修正与热损失表、渗透过程、设计工况汇总、
 * 热平衡迭代结果与两张敏感性实验表写入 heat_load_report.txt。
 * 各数据在本函数内重新计算，与菜单执行顺序无关，保证报表自洽。
 */
void writeReport(void)
{
    FILE *fp = fopen("heat_load_report.txt", "w");
    if (fp == NULL) {
        printf("[错误] 无法创建 heat_load_report.txt。\n");
        return;
    }

    /* 重新计算设计工况 */
    double qEnv  = heatLossFace(tn);
    double qLeak = leakLoss(tn);
    double qTotal = qEnv + qLeak;

    double roomArea = 0.0;
    for (int i = 0; i < MAX_FACE; i++)
        if (g_face[i].valid && g_face[i].orient == 'U') roomArea = g_face[i].area;
    if (roomArea <= 0.0)
        for (int i = 0; i < MAX_FACE; i++)
            if (g_face[i].valid && g_face[i].orient == 'R') roomArea = g_face[i].area;
    if (roomArea <= 0.0)
        for (int i = 0; i < MAX_FACE; i++)
            if (g_face[i].valid) roomArea += g_face[i].area;

    double indicator = (roomArea > 0.0) ? qTotal / roomArea : 0.0;
    double teq = iterEquilibrium(g_wx.qsup, 0);

    /* 一、参数清单 */
    fprintf(fp, "====== 房间冬季采暖负荷计算报表 ======\n\n");
    fprintf(fp, "【一、参数清单】\n");
    fprintf(fp, "室外计算温度 tw = %.1f C   设定室温 tn = %.1f C\n", g_wx.tw, tn);
    fprintf(fp, "朝向修正系数：北 %.2f  东西 %.2f  南 %.2f\n",
            g_wx.alphaN, g_wx.alphaEW, g_wx.alphaS);
    fprintf(fp, "每米缝隙渗风量 = %.2f m3/(m*h)\n", g_wx.leak);
    fprintf(fp, "北窗缝隙总长 = %.2f m   南窗缝隙总长 = %.2f m\n", g_wx.gapN, g_wx.gapS);
    fprintf(fp, "散热器供热能力 = %.0f W\n\n", g_wx.qsup);

    /* 二、逐面修正与热损失表 */
    fprintf(fp, "【二、逐面修正与热损失表】\n");
    fprintf(fp, "名称    朝向  面积m2   K原始  alpha  K修正   热损失W   占比\n");
    for (int i = 0; i < MAX_FACE; i++) {
        if (g_face[i].valid) {
            double alpha = (g_face[i].k != 0.0) ? g_face[i].kAdj / g_face[i].k : 1.0;
            double pct = (qEnv > 0.0) ? g_face[i].q / qEnv * 100.0 : 0.0;
            fprintf(fp, "%-8s  %c   %6.1f   %5.2f  %4.2f  %5.2f   %8.1f   %5.1f%%\n",
                    g_face[i].name, g_face[i].orient, g_face[i].area,
                    g_face[i].k, alpha, g_face[i].kAdj, g_face[i].q, pct);
        }
    }
    fprintf(fp, "\n");

    /* 三、冷风渗透计算过程 */
    fprintf(fp, "【三、冷风渗透计算过程】\n");
    fprintf(fp, "北窗渗风 %.2f m3/h  南窗渗风 %.2f m3/h\n",
            g_wx.leak * g_wx.gapN, g_wx.leak * g_wx.gapS);
    fprintf(fp, "渗透热损失 = %.2f W\n\n", qLeak);

    /* 四、设计工况汇总 */
    fprintf(fp, "【四、设计工况汇总】\n");
    fprintf(fp, "围护传热热损失 %.2f W   渗透热损失 %.2f W\n", qEnv, qLeak);
    fprintf(fp, "总负荷 %.2f W   房间面积 %.2f m2   单位面积指标 %.2f W/m2\n",
            qTotal, roomArea, indicator);
    if (indicator >= 50.0 && indicator <= 80.0)
        fprintf(fp, "（经验范围 50~80 W/m2，合理）\n\n");
    else
        fprintf(fp, "（经验范围 50~80 W/m2，请核查参数）\n\n");

    /* 五、热平衡迭代结果 */
    fprintf(fp, "【五、热平衡迭代结果】\n");
    if (teq > -1e8)
        fprintf(fp, "散热器 %.0f W 时平衡室温约 %.1f C\n\n", g_wx.qsup, teq);
    else
        fprintf(fp, "热平衡迭代未收敛\n\n");

    /* 六、室温敏感性实验 */
    fprintf(fp, "【六、室温敏感性实验（14~24 C）】\n");
    fprintf(fp, "室温C   围护热损失W   渗透热损失W   总负荷W\n");
    for (double t = 14.0; t <= 24.0 + 1e-9; t += 2.0) {
        double qe = heatLossFace(t);
        double ql = leakLoss(t);
        fprintf(fp, "%5.1f   %9.2f   %9.2f   %9.2f\n", t, qe, ql, qe + ql);
    }
    fprintf(fp, "\n");

    /* 七、供热能力敏感性实验 */
    fprintf(fp, "【七、供热能力敏感性实验（2000~4000 W）】\n");
    fprintf(fp, "供热能力W   平衡室温C\n");
    for (double q = 2000.0; q <= 4000.0 + 1e-9; q += 500.0) {
        double te = iterEquilibrium(q, 0);
        if (te > -1e8)
            fprintf(fp, "%8.0f   %8.2f\n", q, te);
        else
            fprintf(fp, "%8.0f   不收敛\n", q);
    }

    fclose(fp);

    /* 恢复设计工况的全局状态 */
    heatLossFace(tn);
    g_qLeak = leakLoss(tn);
    g_qTotal = g_qEnv + g_qLeak;
    g_tn = teq;

    printf("报表已写入 heat_load_report.txt\n");
}
