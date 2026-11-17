#include "Report.h"
#include "KCorrect.h"
#include "HeatLoss.h"
#include "Infiltration.h"
#include "Equilibrium.h"
#include "SumLoad.h"
#include "Sensitivity.h"

/* M9 负荷报表输出模块
 *
 * 设计要点：报表里的所有数字都在本函数内用设计室温重算一遍，
 * 因此无论用户按什么顺序点菜单（甚至跳过 M7、M8），报表始终自洽。
 * 报表结构与屏幕输出共用同一批函数（heatLossFace / leakLoss / alphaForOrient /
 * sensitivityTemps / sensitivityCapacity），不会出现"屏幕是一套数、文件是另一套数"。
 */

/* 一、参数清单 */
static void writeParams(FILE *fp)
{
    fprintf(fp, "【一、输入参数】\n");
    fprintf(fp, "室外计算温度 tw = %.1f C   设计室温 tn = %.1f C\n",
            g_wx.tw, g_tnDesign);
    fprintf(fp, "朝向修正系数：北 %.2f  东西 %.2f  南 %.2f（屋面/地面不修正，取 1.00）\n",
            g_wx.alphaN, g_wx.alphaEW, g_wx.alphaS);
    fprintf(fp, "每米缝隙渗风量 L = %.2f m3/(m*h)\n", g_wx.leak);
    fprintf(fp, "北窗缝隙总长 = %.2f m   南窗缝隙总长 = %.2f m\n",
            g_wx.gapN, g_wx.gapS);
    fprintf(fp, "散热器供热能力 Qsup = %.0f W\n\n", g_wx.qsup);
}

/* 二、逐面修正与热损失表 */
static void writeFaceTable(FILE *fp, double qEnv)
{
    static const char *const head[] = { "朝向", "面积m2", "K原始", "alpha",
                                        "K修正", "温差C", "热损失W", "占比" };

    fprintf(fp, "【二、逐面传热系数修正与热损失】\n");
    fprintPadded(fp, "名称", 10);
    fprintHeader(fp, head, (int)(sizeof head / sizeof head[0]));

    for (int i = 0; i < g_faceCnt; i++) {
        double pct;
        if (!g_face[i].valid) continue;

        pct = (qEnv > 0.0) ? g_face[i].q / qEnv * 100.0 : 0.0;
        fprintPadded(fp, g_face[i].name, 10);
        fprintf(fp, "  %-6c  %6.1f  %6.2f  %6.2f  %6.2f  %6.1f  %6.1f  %5.1f%%\n",
                g_face[i].orient, g_face[i].area, g_face[i].k,
                alphaForOrient(g_face[i].orient), g_face[i].kAdj,
                g_tnDesign - g_wx.tw, g_face[i].q, pct);
    }
    fprintf(fp, "（合计 g_qEnv = %.2f W）\n\n", qEnv);
}

/* 三、冷风渗透计算过程 */
static void writeInfiltration(FILE *fp, double qLeak)
{
    double flowN = leakFlowN();
    double flowS = leakFlowS();

    fprintf(fp, "【三、冷风渗透计算过程】\n");
    fprintf(fp, "北窗：渗风量 %.2f m3/h  质量流量 %.2f kg/h\n", flowN, RHO_AIR * flowN);
    fprintf(fp, "南窗：渗风量 %.2f m3/h  质量流量 %.2f kg/h\n", flowS, RHO_AIR * flowS);
    fprintf(fp, "渗透总系数 C = %.4f W/K\n", leakCoeff());
    fprintf(fp, "渗透热损失 Q_渗 = C·(tn-tw) = %.2f W\n\n", qLeak);
}

/* 四、设计工况汇总 */
static void writeDesignSummary(FILE *fp, double qEnv, double qLeak, double qTotal)
{
    double area = roomArea();
    double indicator = loadIndicator(qTotal);

    fprintf(fp, "【四、设计工况汇总】\n");
    fprintf(fp, "围护传热热损失 %.2f W   渗透热损失 %.2f W\n", qEnv, qLeak);
    fprintf(fp, "总负荷 %.2f W   房间面积 %.2f m2   单位面积指标 %.2f W/m2\n",
            qTotal, area, indicator);

    if (area <= 0.0) {
        fprintf(fp, "（无有效围护面，房间面积与负荷指标无法计算）\n\n");
    } else if (indicator >= IDX_LO && indicator <= IDX_HI) {
        fprintf(fp, "（经验范围 %.0f~%.0f W/m2，合理）\n\n", IDX_LO, IDX_HI);
    } else if (indicator < IDX_LO) {
        fprintf(fp, "（经验范围 %.0f~%.0f W/m2，偏低，请核查围护参数与房间面积）\n\n",
                IDX_LO, IDX_HI);
    } else {
        fprintf(fp, "（经验范围 %.0f~%.0f W/m2，偏高，请核查围护参数与房间面积）\n\n",
                IDX_LO, IDX_HI);
    }
}

/* 五、热平衡迭代结果 */
static void writeEquilibrium(FILE *fp, double teq)
{
    fprintf(fp, "【五、热平衡迭代结果】\n");
    fprintf(fp, "求解方程：Qsup = (S + C)·(tn - tw)，S = Σ(K'·A) = %.2f W/K，C = %.4f W/K\n",
            envCoeff(), leakCoeff());
    if (ITER_OK(teq)) {
        fprintf(fp, "散热器 %.0f W 时平衡室温约 %.2f C（迭代 %d 轮，判据 %.2f C）\n",
                g_wx.qsup, teq, g_iterCnt, ITER_EPS);
        printCapacityNote(fp, g_wx.qsup, teq);
    } else if (envCoeff() <= 0.0) {
        fprintf(fp, "围护总传热能力 S = 0（无有效围护面），热平衡温度无解。\n");
    } else {
        fprintf(fp, "热平衡迭代未收敛：请检查围护面数据与散热器供热能力。\n");
    }
}

void writeReport(void)
{
    FILE *fp = fopen(REPORT_FILE, "w");
    if (fp == NULL) {
        printf("[错误] 无法创建 %s（文件被占用或目录不可写）。\n", REPORT_FILE);
        return;
    }

    /* 用设计室温重算，保证报表与菜单执行顺序无关 */
    double qEnv   = heatLossFace(g_tnDesign);
    double qLeak  = leakLoss(g_tnDesign);
    double qTotal = qEnv + qLeak;
    double teq    = iterEquilibrium(g_wx.qsup, 0);

    fprintf(fp, "====== 房间冬季采暖负荷计算报表 ======\n");
    fprintf(fp, "（本报表由设计室温 %.1f C 重新计算生成，与菜单操作顺序无关）\n\n",
            g_tnDesign);

    writeParams(fp);
    writeFaceTable(fp, qEnv);
    writeInfiltration(fp, qLeak);
    writeDesignSummary(fp, qEnv, qLeak, qTotal);
    writeEquilibrium(fp, teq);

    /* 六、七两节直接复用 M8 的表格函数，报表与屏幕内容天然一致 */
    fprintf(fp, "\n【六、室温敏感性实验（%.0f~%.0f C）】\n",
            SENS_TN_BEGIN, SENS_TN_END);
    sensitivityTemps(fp);
    fprintf(fp, "\n【七、供热能力敏感性实验（%.0f~%.0f W）】\n",
            SENS_Q_BEGIN, SENS_Q_END);
    sensitivityCapacity(fp);

    if (fclose(fp) != 0) {
        printf("[错误] 写入 %s 时出错，请检查磁盘空间。\n", REPORT_FILE);
        return;
    }

    /* 同步全局状态：报表里的数值已成为当前设计工况结果 */
    refreshDesignLoad();
    if (ITER_OK(teq)) g_tn = teq;

    printf("报表已写入 %s\n", REPORT_FILE);
}
