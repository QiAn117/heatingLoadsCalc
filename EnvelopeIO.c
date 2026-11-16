#include "EnvelopeIO.h"
#include <string.h>

/* M1 围护结构参数文件读入模块
 * 文件格式：首行为表头，其余每行 "名称,朝向代码,面积m2,传热系数W/(m2K)"
 * 校验：面积 1~50、K 0.2~3.5、朝向代码 N/S/E/W/R/U；
 *       非法行 valid=0，不参与后续计算，并打印警告。
 */
void loadEnvelope(void)
{
    FILE *fp = fopen("envelope.txt", "r");
    if (fp == NULL) {
        printf("[错误] 无法打开 envelope.txt，请确认文件在当前目录。\n");
        return;
    }

    /* 跳过 UTF-8 BOM（如有） */
    int c = fgetc(fp);
    if (c == 0xEF) { fgetc(fp); fgetc(fp); }   /* EF BB BF */
    else ungetc(c, fp);

    char line[256];
    int warnCnt = 0;
    g_faceCnt = 0;

    /* 表头行 */
    if (fgets(line, sizeof line, fp) == NULL) {
        printf("[错误] envelope.txt 为空。\n");
        fclose(fp);
        return;
    }

    while (fgets(line, sizeof line, fp) != NULL && g_faceCnt < MAX_FACE) {
        size_t n = strlen(line);
        while (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r')) line[--n] = '\0';
        if (n == 0) continue;   /* 空行跳过 */

        struct Envelope e;
        char orient = 0;
        double area = 0.0, k = 0.0;
        int ok = sscanf(line, "%15[^,], %c,%lf,%lf", e.name, &orient, &area, &k);

        if (ok != 4) {
            printf("[警告] 行格式错误，已忽略：%s\n", line);
            warnCnt++;
            continue;
        }

        int valid = 1;
        if (strchr("NSEWRU", orient) == NULL) {
            printf("[警告] 朝向代码非法 '%c'（应为 N/S/E/W/R/U），本行无效：%s\n", orient, line);
            valid = 0;
        }
        if (area < 1.0 || area > 50.0) {
            printf("[警告] 面积 %.1f 超出范围（1~50），本行无效：%s\n", area, line);
            valid = 0;
        }
        if (k < 0.2 || k > 3.5) {
            printf("[警告] 传热系数 %.2f 超出范围（0.2~3.5），本行无效：%s\n", k, line);
            valid = 0;
        }
        if (!valid) warnCnt++;

        e.orient = orient;
        e.area = area;
        e.k = k;
        e.kAdj = 0.0;
        e.q = 0.0;
        e.valid = valid;
        g_face[g_faceCnt++] = e;
    }
    fclose(fp);

    int validCnt = 0;
    for (int i = 0; i < g_faceCnt; i++)
        if (g_face[i].valid) validCnt++;

    printf("已读入围护面 %d 个（有效 %d 个，警告 %d 条）\n", g_faceCnt, validCnt, warnCnt);
}
