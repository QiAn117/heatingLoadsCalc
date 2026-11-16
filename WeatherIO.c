#include "WeatherIO.h"
#include <string.h>
#include <stdlib.h>

/* M2 室外气象参数读入模块
 * 文件格式："关键字,数值" 每行一条，按关键字子串匹配后存入 g_wx。
 * 校验：室外计算温度必须为负；若为正（丢失负号），自动取负并警告。
 */
void loadWeather(void)
{
    FILE *fp = fopen("weather.txt", "r");
    if (fp == NULL) {
        printf("[错误] 无法打开 weather.txt，请确认文件在当前目录。\n");
        return;
    }

    int c = fgetc(fp);
    if (c == 0xEF) { fgetc(fp); fgetc(fp); }
    else ungetc(c, fp);

    char line[256];
    while (fgets(line, sizeof line, fp) != NULL) {
        size_t n = strlen(line);
        while (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r')) line[--n] = '\0';
        if (n == 0) continue;

        char *comma = strchr(line, ',');
        if (comma == NULL) {
            printf("[警告] 行格式错误（缺少逗号），已忽略：%s\n", line);
            continue;
        }
        *comma = '\0';
        const char *key = line;
        double value = atof(comma + 1);

        if      (strstr(key, "tw_C"))   g_wx.tw = value;
        else if (strstr(key, "北向"))    g_wx.alphaN = value;
        else if (strstr(key, "东西向"))  g_wx.alphaEW = value;
        else if (strstr(key, "南向"))    g_wx.alphaS = value;
        else if (strstr(key, "L_m3hm")) g_wx.leak = value;
        else if (strstr(key, "北窗"))    g_wx.gapN = value;
        else if (strstr(key, "南窗"))    g_wx.gapS = value;
        else if (strstr(key, "Qsup_W")) g_wx.qsup = value;
        else printf("[警告] 未知关键字，已忽略：%s\n", key);
    }
    fclose(fp);

    /* 校验：室外温度必须为负 */
    if (g_wx.tw >= 0.0) {
        printf("[警告] 室外计算温度 %.1f 疑似录入错误（丢失负号），已自动取负。\n", g_wx.tw);
        g_wx.tw = -g_wx.tw;
    }

    /* 回显 */
    printf("=== 室外气象参数回显 ===\n");
    printf("室外计算温度 tw = %.1f C\n", g_wx.tw);
    printf("朝向修正：北 %.2f  东西 %.2f  南 %.2f\n", g_wx.alphaN, g_wx.alphaEW, g_wx.alphaS);
    printf("每米缝隙渗风量 = %.2f m3/(m*h)\n", g_wx.leak);
    printf("北窗缝隙总长 = %.2f m   南窗缝隙总长 = %.2f m\n", g_wx.gapN, g_wx.gapS);
    printf("散热器供热能力 = %.0f W\n", g_wx.qsup);
}
