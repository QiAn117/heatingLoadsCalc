#include "EnvelopeIO.h"

/* M1 围护结构参数文件读入模块
 *
 * 文件格式：首行可为表头，其余每行 "名称,朝向代码,面积m2,传热系数W/(m2K)"
 * 校验：面积 1~50、K 0.2~3.5、朝向代码 N/S/E/W/R/U；
 *       非法行保留在 g_face[] 中但置 valid=0，不参与后续计算，并打印警告。
 * 说明：表头为"可选"，首行若解析不出 4 个字段就按表头跳过（见 firstLine 处理），
 *       这样既兼容带表头的样例文件，也兼容只有数据行的文件。
 */

/* 解析一行围护面记录：成功返回 4，否则返回 sscanf 已匹配到的字段数。
 * sscanf 的 " %c" 前留一个空格，可跳过逗号后的空白。 */
static int parseFaceRow(const char *line, struct Envelope *e)
{
    char   orient = 0;
    double area = 0.0;
    double k = 0.0;
    int    ok = sscanf(line, "%15[^,], %c,%lf,%lf", e->name, &orient, &area, &k);

    if (ok == 4) {
        e->orient = orient;
        e->area   = area;
        e->k      = k;
    }
    return ok;
}

int loadEnvelope(void)
{
    FILE *fp = fopen(ENVELOPE_FILE, "r");
    if (fp == NULL) {
        printf("[错误] 无法打开 %s，请确认文件在当前目录。\n", ENVELOPE_FILE);
        return 0;
    }
    skipUtf8Bom(fp);

    char line[256];
    int  warnCnt   = 0;      /* 警告条数 */
    int  extraCnt  = 0;      /* 超出 MAX_FACE 被忽略的行数 */
    int  firstLine = 1;      /* 是否还在处理第一行（可能是表头） */

    g_faceCnt = 0;

    while (fgets(line, sizeof line, fp) != NULL) {
        struct Envelope e;
        int    ok;
        int    valid = 1;

        trim(line);
        if (line[0] == '\0') continue;          /* 空行跳过 */

        ok = parseFaceRow(line, &e);

        if (firstLine) {
            firstLine = 0;
            if (ok != 4) continue;              /* 首行不是数据行：当作表头跳过 */
        }
        if (ok != 4) {
            printf("[警告] 行格式错误，已忽略：%s\n", line);
            warnCnt++;
            continue;
        }

        if (g_faceCnt >= MAX_FACE) {            /* 超限：不写入数组，只统计 */
            extraCnt++;
            continue;
        }

        /* 名称长度检查：%15[^,] 最多读 15 字节，超出部分会被 sscanf 截断 */
        {
            const char *comma = strchr(line, ',');
            if (comma != NULL && comma - line > 15)
                printf("[警告] 面名称超过 15 字节，已截断：%s\n", e.name);
        }

        if (strchr("NSEWRU", e.orient) == NULL) {
            printf("[警告] 朝向代码非法 '%c'（应为 N/S/E/W/R/U），本行无效：%s\n",
                   e.orient, line);
            valid = 0;
        }
        if (e.area < AREA_MIN || e.area > AREA_MAX) {
            printf("[警告] 面积 %.2f 超出范围（%.0f~%.0f），本行无效：%s\n",
                   e.area, AREA_MIN, AREA_MAX, line);
            valid = 0;
        }
        if (e.k < K_MIN || e.k > K_MAX) {
            printf("[警告] 传热系数 %.2f 超出范围（%.1f~%.1f），本行无效：%s\n",
                   e.k, K_MIN, K_MAX, line);
            valid = 0;
        }
        if (!valid) warnCnt++;

        e.kAdj  = 0.0;
        e.q     = 0.0;
        e.valid = valid;
        g_face[g_faceCnt++] = e;
    }
    fclose(fp);

    if (extraCnt > 0) {
        printf("[警告] 围护面超过上限 %d 个，后面的 %d 行已忽略。\n", MAX_FACE, extraCnt);
        warnCnt++;
    }
    if (g_faceCnt == 0) {
        printf("[错误] %s 中没有读到任何围护面数据。\n", ENVELOPE_FILE);
        return 0;
    }

    int validCnt = 0;
    for (int i = 0; i < g_faceCnt; i++)
        if (g_face[i].valid) validCnt++;

    printf("已读入围护面 %d 个（有效 %d 个，警告 %d 条）\n",
           g_faceCnt, validCnt, warnCnt);
    if (validCnt == 0)
        printf("[提示] 全部围护面均无效，负荷计算将得到 0 W，请检查 %s。\n",
               ENVELOPE_FILE);

    return 1;
}
