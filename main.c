#include <stdio.h>
#include "common.h"

/* 全局数据 */
struct Envelope g_face[MAX_FACE];
struct Weather  g_wx;
int    g_faceCnt;        /* 围护面数 */
double g_tn;             /* 设定/平衡室温 C，迭代求出 */
double g_qEnv, g_qLeak, g_qTotal; /* 围护热损失/渗透热损失/总负荷 W */

int main(void)
{

    return 0;
}