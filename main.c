#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* M10 主控菜单模块
 * 职责：
 *   1) 维护"数据是否读入 / 系数是否修正"的状态，未就绪时拒绝计算；
 *   2) 负责全部交互输入（读整行再解析，避免 scanf 失败后残留字符导致菜单乱跳）；
 *   3) 用 do-while/switch 调度 M1~M9。
 * 状态约束：M3 前必须完成 M1、M2；M4~M9 依赖 M3 的修正结果。
 */

/* 全局数据定义 */
struct Envelope g_face[MAX_FACE];
struct Weather  g_wx = { -19.0, 1.10, 1.00, 0.85, 0.90, 7.2, 7.2, 2600.0 };
int    g_faceCnt  = 0;
double g_tnDesign = TN_DESIGN_DEFAULT;
double g_tn       = 0.0;
int    g_iterCnt  = 0;
double g_qEnv     = 0.0;
double g_qLeak    = 0.0;
double g_qTotal   = 0.0;

/* 菜单返回值中的两个特殊值 */
#define MENU_EOF     (-2)    /* 标准输入结束 */
#define MENU_INVALID (-1)    /* 非法输入 */

static int readLine(char *buf, int size)
{
    if (fgets(buf, size, stdin) == NULL) return 0;

    if (strchr(buf, '\n') == NULL && !feof(stdin)) 
    {   /* 超长行：丢弃剩余字符 */
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
    trim(buf);
    return 1;
}

/* 读取菜单选项：空行/非数字/小数返回 MENU_INVALID */
static int readChoice(void)
{
    char   buf[64];
    double v;

    if (!readLine(buf, sizeof buf)) return MENU_EOF;
    if (buf[0] == '\0') return MENU_INVALID;
    if (!parseDouble(buf, &v) || v < -100.0 || v > 100.0) return MENU_INVALID;
    if (v != (double)(int)v) return MENU_INVALID;
    return (int)v;
}

/* 交互设定设计室温，写入 g_tnDesign；返回 0 表示遇到 EOF。
 * 直接回车沿用上次的值，避免"必须重新输一遍 18"的繁琐。 */
static int askDesignTemp(void)
{
    char   buf[64];
    double v;

    printf("请输入设计室温 tn (C，直接回车沿用 %.1f)：", g_tnDesign);
    if (!readLine(buf, sizeof buf)) return 0;
    if (buf[0] == '\0') return 1;

    if (!parseDouble(buf, &v)) {
        printf("[提示] 输入无效，沿用 %.1f C。\n", g_tnDesign);
        return 1;
    }
    if (v < TN_MIN || v > TN_MAX) {
        printf("[提示] 室温应在 %.0f~%.0f C 之间，沿用 %.1f C。\n",
               TN_MIN, TN_MAX, g_tnDesign);
        return 1;
    }
    g_tnDesign = v;
    return 1;
}

/* 清空上一次的计算结果：重新读入数据后，旧的负荷/平衡温度不再有效 */
static void resetResults(void)
{
    g_qEnv    = 0.0;
    g_qLeak   = 0.0;
    g_qTotal  = 0.0;
    g_tn      = 0.0;
    g_iterCnt = 0;
}

static void printMenu(int envelopeLoaded, int weatherLoaded, int kAdjusted)
{
    printf("\n---------------- 主菜单 ----------------\n");
    printf(" 1. 读入围护结构参数表 (M1)\n");
    printf(" 2. 读入室外气象参数 (M2)\n");
    printf(" 3. 传热系数修正 (M3)\n");
    printf(" 4. 围护与渗透热损失计算 (M4+M5)\n");
    printf(" 5. 热平衡温度迭代 (M6)\n");
    printf(" 6. 采暖负荷汇总与指标 (M7)\n");
    printf(" 7. 室温与供热能力敏感性实验 (M8)\n");
    printf(" 8. 负荷报表输出 (M9)\n");
    printf(" 0. 退出\n");
    printf("状态：围护表[%s]  气象参数[%s]  系数修正[%s]  设计室温 %.1f C\n",
           envelopeLoaded ? "已读" : "未读",
           weatherLoaded  ? "已读" : "未读",
           kAdjusted      ? "已完成" : "未完成",
           g_tnDesign);
    printf("请选择：");
}

/* 计算类模块的统一前置检查 */
static int ensureReady(int kAdjusted)
{
    if (kAdjusted) return 1;
    printf("[提示] 请先执行 1、2、3 完成数据读入与系数修正。\n");
    return 0;
}

int main(void)
{
#ifdef _WIN32
    system("chcp 65001 > NUL");   /* 控制台切到 UTF-8，保证中文正常显示 */
#endif

    int envelopeLoaded = 0;   /* M1 完成 */
    int weatherLoaded  = 0;   /* M2 完成 */
    int kAdjusted      = 0;   /* M3 完成 */
    int running        = 1;

    printf("====== 房间冬季采暖负荷计算系统 ======\n");
    printf("题目 A4：朝向修正 → 逐面累加 → 迭代求热平衡 → 敏感性分析\n");

    while (running) {
        int choice;

        printMenu(envelopeLoaded, weatherLoaded, kAdjusted);
        choice = readChoice();

        switch (choice) {
        case MENU_EOF:
            printf("\n[提示] 输入结束，程序退出。\n");
            running = 0;
            break;

        case 1:
            /* 只有真正读入成功才置标志：原实现不管成败都置 1，
             * 文件不存在时仍能继续算出一堆 0，属于"假成功"。 */
            if (loadEnvelope()) {
                envelopeLoaded = 1;
                kAdjusted = 0;
                resetResults();
            }
            break;

        case 2:
            if (loadWeather()) {
                weatherLoaded = 1;
                kAdjusted = 0;      /* 朝向修正系数可能变了，kAdj 必须重算 */
                resetResults();
            }
            break;

        case 3:
            if (!envelopeLoaded || !weatherLoaded) {
                printf("[提示] 请先执行 1、2 读入两个数据文件。\n");
            } else {
                kCorrect();
                kAdjusted = 1;
            }
            break;

        case 4:
            if (!ensureReady(kAdjusted)) break;
            if (!askDesignTemp()) { printf("\n[提示] 输入结束，程序退出。\n"); running = 0; break; }
            heatlossCalc(g_tnDesign);
            infiltrationCalc(g_tnDesign);
            printf("\n[提示] 当前总负荷 = %.2f W（如需指标汇总请执行 6）。\n",
                   g_qEnv + g_qLeak);
            break;

        case 5:
            if (!ensureReady(kAdjusted)) break;
            equilibrium();
            break;

        case 6:
            if (!ensureReady(kAdjusted)) break;
            if (!askDesignTemp()) { printf("\n[提示] 输入结束，程序退出。\n"); running = 0; break; }
            sumLoad();
            break;

        case 7:
            if (!ensureReady(kAdjusted)) break;
            sensitivity();
            break;

        case 8:
            if (!ensureReady(kAdjusted)) break;
            writeReport();
            break;

        case 0:
            printf("程序退出。\n");
            running = 0;
            break;

        default:
            printf("[提示] 无效选项，请输入 0~8。\n");
        }
    }

    return 0;
}
