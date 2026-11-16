#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "EnvelopeIO.h"
#include "WeatherIO.h"
#include "KCorrect.h"
#include "HeatLoss.h"
#include "Infiltration.h"
#include "Equilibrium.h"
#include "SumLoad.h"
#include "Sensitivity.h"
#include "Report.h"

/* 全局数据定义 */
struct Envelope g_face[MAX_FACE];
struct Weather  g_wx;
int    g_faceCnt = 0;        /* 围护面数 */
double tn = 18.0;            /* 设定室温（设计工况默认 18C） */
double g_tn = 0.0;           /* 平衡室温 C，由 M6 迭代求出 */
double g_qEnv = 0.0, g_qLeak = 0.0, g_qTotal = 0.0;

int main(void)
{
    system("chcp 65001 > NUL");   /* 控制台切到 UTF-8，保证中文正常显示 */

    int choice = -1;
    int dataLoaded = 0;   /* M1 完成 */
    int wxLoaded = 0;     /* M2 完成 */
    int kFixed = 0;       /* M3 完成 */

    printf("====== 房间采暖负荷计算系统 ======\n");

    do {
        printf("\n---------------- 主菜单 ----------------\n");
        printf(" 1. 读入围护结构参数表 (M1)\n");
        printf(" 2. 读入室外气象参数 (M2)\n");
        printf(" 3. 传热系数修正 (M3)\n");
        printf(" 4. 围护与渗透热损失计算 (M4+M5)\n");
        printf(" 5. 热平衡温度迭代 (M6)\n");
        printf(" 6. 采暖负荷汇总与指标 (M7)\n");
        printf(" 7. 室温敏感性实验 (M8)\n");
        printf(" 8. 负荷报表输出 (M9)\n");
        printf(" 0. 退出\n");
        printf("请选择：");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');   /* 清空错误输入 */
            choice = -1;
        }

        switch (choice) {
        case 1:
            loadEnvelope();
            dataLoaded = 1;
            break;
        case 2:
            loadWeather();
            wxLoaded = 1;
            break;
        case 3:
            if (!dataLoaded || !wxLoaded)
                printf("[提示] 请先执行 1、2 读入两个数据文件。\n");
            else {
                kCorrect();
                kFixed = 1;
            }
            break;
        case 4:
            if (!kFixed) {
                printf("[提示] 请先执行 1、2、3 完成读入与系数修正。\n");
            } else {
                printf("请输入设定室温 tn (C)：");
                if (scanf("%lf", &tn) != 1) { tn = 18.0; while (getchar() != '\n'); }
                heatlossCalc();
                g_qLeak = leakLoss(tn);
                printf("冷风渗透热损失 = %.2f W\n", g_qLeak);
            }
            break;
        case 5:
            if (!kFixed)
                printf("[提示] 请先执行 1、2、3 完成读入与系数修正。\n");
            else
                equilibrium();
            break;
        case 6:
            if (!kFixed)
                printf("[提示] 请先执行 1、2、3 完成读入与系数修正。\n");
            else
                sumLoad();
            break;
        case 7:
            if (!kFixed)
                printf("[提示] 请先执行 1、2、3 完成读入与系数修正。\n");
            else
                sensitivity();
            break;
        case 8:
            if (!kFixed)
                printf("[提示] 请先执行 1、2、3 完成读入与系数修正。\n");
            else
                writeReport();
            break;
        case 0:
            printf("程序退出。\n");
            break;
        default:
            printf("[提示] 无效选项，请重新输入。\n");
        }
    } while (choice != 0);

    return 0;
}
