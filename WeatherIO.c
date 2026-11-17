#include "WeatherIO.h"

/* M2 室外气象参数读入模块
 *
 * 文件格式：每行 "关键字,数值"，按关键字子串匹配后存入 g_wx。
 * 校验内容：
 *   1) 数值必须是合法数字（"abc"、"1.2x" 都会被识别为录入错误）；
 *   2) 室外计算温度必须为负：为正说明丢了负号，自动取负并警告；为 0 则告警；
 *   3) 渗风量/缝隙长/供热能力/朝向修正系数按设计区间检查越界；
 *   4) 关键项缺失时给警告并沿用默认值——原实现缺项时全局变量为 0，
 *      会把 qsup=0 代进迭代，算出 -19 C 这种看似"收敛"的荒谬结果。
 */

/* 缺项检查：每读到一个关键字就置位，读完后统一报告 */
enum {
    KEY_TW = 1 << 0,
    KEY_ALPHA_N = 1 << 1,
    KEY_ALPHA_EW = 1 << 2,
    KEY_ALPHA_S = 1 << 3,
    KEY_LEAK = 1 << 4,
    KEY_GAP_N = 1 << 5,
    KEY_GAP_S = 1 << 6,
    KEY_QSUP = 1 << 7
};

/* 关键字子串匹配后把数值写入对应成员，返回该关键字对应的标志位；未知关键字返回 0 */
static int storeValue(const char *key, double value)
{
    if      (strstr(key, "tw_C")   != NULL) { g_wx.tw      = value; return KEY_TW; }
    else if (strstr(key, "北向")   != NULL) { g_wx.alphaN  = value; return KEY_ALPHA_N; }
    else if (strstr(key, "东西向") != NULL) { g_wx.alphaEW = value; return KEY_ALPHA_EW; }
    else if (strstr(key, "南向")   != NULL) { g_wx.alphaS  = value; return KEY_ALPHA_S; }
    else if (strstr(key, "L_m3hm") != NULL) { g_wx.leak    = value; return KEY_LEAK; }
    else if (strstr(key, "北窗")   != NULL) { g_wx.gapN    = value; return KEY_GAP_N; }
    else if (strstr(key, "南窗")   != NULL) { g_wx.gapS    = value; return KEY_GAP_S; }
    else if (strstr(key, "Qsup_W") != NULL) { g_wx.qsup    = value; return KEY_QSUP; }
    return 0;
}

/* 缺项提示：返回警告条数 */
static int reportMissing(int found)
{
    int n = 0;

    if (!(found & KEY_TW)) {
        printf("[警告] 缺少室外计算温度 tw_C，暂用 %.1f C。\n", g_wx.tw);
        n++;
    }
    if (!(found & KEY_ALPHA_N)) {
        printf("[警告] 缺少北向朝向修正，暂用 %.2f。\n", g_wx.alphaN);
        n++;
    }
    if (!(found & KEY_ALPHA_EW)) {
        printf("[警告] 缺少东西向朝向修正，暂用 %.2f。\n", g_wx.alphaEW);
        n++;
    }
    if (!(found & KEY_ALPHA_S)) {
        printf("[警告] 缺少南向朝向修正，暂用 %.2f。\n", g_wx.alphaS);
        n++;
    }
    if (!(found & KEY_LEAK)) {
        printf("[警告] 缺少每米缝隙渗风量 L_m3hm，暂用 %.2f m3/(m*h)。\n", g_wx.leak);
        n++;
    }
    if (!(found & KEY_GAP_N)) {
        printf("[警告] 缺少北窗缝隙总长，暂用 %.2f m。\n", g_wx.gapN);
        n++;
    }
    if (!(found & KEY_GAP_S)) {
        printf("[警告] 缺少南窗缝隙总长，暂用 %.2f m。\n", g_wx.gapS);
        n++;
    }
    if (!(found & KEY_QSUP)) {
        printf("[警告] 缺少散热器供热能力 Qsup_W，暂用 %.0f W。\n", g_wx.qsup);
        n++;
    }
    return n;
}

/* 区间校验：越界只提示不修改，避免悄悄改掉用户数据 */
static int checkRange(const char *what, double v, double lo, double hi)
{
    if (v < lo || v > hi) {
        printf("[警告] %s = %.3f 超出建议范围（%.1f~%.1f）。\n", what, v, lo, hi);
        return 1;
    }
    return 0;
}

int loadWeather(void)
{
    /* 先放一组与题目样例一致的默认值：缺项时程序仍能跑完，并有警告可查 */
    g_wx.tw      = -19.0;
    g_wx.alphaN  = 1.10;
    g_wx.alphaEW = 1.00;
    g_wx.alphaS  = 0.85;
    g_wx.leak    = 0.90;
    g_wx.gapN    = 7.2;
    g_wx.gapS    = 7.2;
    g_wx.qsup    = 2600.0;

    FILE *fp = fopen(WEATHER_FILE, "r");
    if (fp == NULL) {
        printf("[错误] 无法打开 %s，请确认文件在当前目录。\n", WEATHER_FILE);
        return 0;
    }
    skipUtf8Bom(fp);

    char line[256];
    int  found   = 0;
    int  warnCnt = 0;

    while (fgets(line, sizeof line, fp) != NULL) {
        char  *comma;
        char  *key;
        double value;

        trim(line);
        if (line[0] == '\0') continue;

        comma = strchr(line, ',');
        if (comma == NULL) {
            printf("[警告] 行格式错误（缺少逗号），已忽略：%s\n", line);
            warnCnt++;
            continue;
        }
        *comma = '\0';
        key = trim(line);

        if (!parseDouble(comma + 1, &value)) {
            printf("[警告] 关键字 \"%s\" 的数值无法解析，已忽略：%s\n", key, comma + 1);
            warnCnt++;
            continue;
        }

        int flag = storeValue(key, value);
        if (flag == 0) {
            printf("[警告] 未知关键字，已忽略：%s\n", key);
            warnCnt++;
        } else {
            found |= flag;
        }
    }
    fclose(fp);

    warnCnt += reportMissing(found);

    /* 室外温度符号纠正：为正=丢了负号；为 0=数据可疑。
     * 注意必须先判 >0 再判 ==0，否则 0 会被取负得到 -0.0，显示成 "-0.0 C"。 */
    if (g_wx.tw > 0.0) {
        printf("[警告] 室外计算温度 %.1f 为正，疑似丢失负号，已自动取负。\n", g_wx.tw);
        g_wx.tw = -g_wx.tw;
        warnCnt++;
    } else if (g_wx.tw == 0.0) {
        printf("[警告] 室外计算温度为 0 C，不符合采暖室外计算温度的定义，请检查 %s。\n",
               WEATHER_FILE);
        warnCnt++;
    }

    /* 区间校验 */
    warnCnt += checkRange("每米缝隙渗风量", g_wx.leak,   LEAK_MIN,  LEAK_MAX);
    warnCnt += checkRange("北窗缝隙总长",   g_wx.gapN,   GAP_MIN,   GAP_MAX);
    warnCnt += checkRange("南窗缝隙总长",   g_wx.gapS,   GAP_MIN,   GAP_MAX);
    warnCnt += checkRange("散热器供热能力", g_wx.qsup,   QSUP_MIN,  QSUP_MAX);
    warnCnt += checkRange("北向朝向修正",   g_wx.alphaN, ALPHA_MIN, ALPHA_MAX);
    warnCnt += checkRange("东西向朝向修正", g_wx.alphaEW, ALPHA_MIN, ALPHA_MAX);
    warnCnt += checkRange("南向朝向修正",   g_wx.alphaS, ALPHA_MIN, ALPHA_MAX);

    /* 回显 */
    printf("=== 室外气象参数回显（警告 %d 条） ===\n", warnCnt);
    printf("室外计算温度 tw = %.1f C\n", g_wx.tw);
    printf("朝向修正：北 %.2f  东西 %.2f  南 %.2f\n",
           g_wx.alphaN, g_wx.alphaEW, g_wx.alphaS);
    printf("每米缝隙渗风量 = %.2f m3/(m*h)\n", g_wx.leak);
    printf("北窗缝隙总长 = %.2f m   南窗缝隙总长 = %.2f m\n", g_wx.gapN, g_wx.gapS);
    printf("散热器供热能力 = %.0f W\n", g_wx.qsup);

    return 1;
}
