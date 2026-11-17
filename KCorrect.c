#include "KCorrect.h"

/* M3 传热系数修正计算模块
 * 遍历 g_face[]，按朝向取 alpha，计算 kAdj = k * alpha 并回写，
 * 打印"原始 K — alpha — 修正 K"对照表（题目要求的逐面修正对照表）。
 */

double alphaForOrient(char orient)
{
    switch (orient) {
    case 'N': return g_wx.alphaN;
    case 'S': return g_wx.alphaS;
    case 'E':
    case 'W': return g_wx.alphaEW;
    case 'R':
    case 'U': return 1.00;          /* 屋面、地面不做朝向修正 */
    default:  return 1.00;          /* 非法朝向已在 M1 被置 valid=0，这里兜底 */
    }
}

void kCorrect(void)
{
    /* 表头列名，列宽由 printHeader 统一处理 */
    static const char *const head[] = { "朝向", "面积m2", "K原始", "alpha", "K修正" };
    int validCnt = 0;

    printf("\n====== 传热系数朝向修正（kAdj = k × alpha） ======\n");
    printPadded("名称", 10);
    printHeader(head, (int)(sizeof head / sizeof head[0]));

    for (int i = 0; i < g_faceCnt; i++) {
        double alpha;
        if (!g_face[i].valid) continue;

        alpha = alphaForOrient(g_face[i].orient);
        g_face[i].kAdj = g_face[i].k * alpha;
        validCnt++;

        printPadded(g_face[i].name, 10);
        printf("  %-6c  %6.1f  %6.2f  %6.2f  %6.2f\n",
               g_face[i].orient, g_face[i].area, g_face[i].k,
               alpha, g_face[i].kAdj);
    }

    if (validCnt == 0)
        printf("[提示] 没有有效围护面，未做任何修正。\n");
    else
        printf("已完成 %d 个围护面的朝向修正。\n", validCnt);
}
