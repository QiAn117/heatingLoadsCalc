#ifndef HEATLOSS_H
#define HEATLOSS_H

#include "common.h"
#include <stdio.h>
extern struct Envelope g_face[MAX_FACE];
extern struct Weather  g_wx;
extern int    g_faceCnt;        /* 围护面数 */
extern double tn;
extern double g_tn;             /* 设定/平衡室温 C，迭代求出 */
extern double g_qEnv, g_qLeak, g_qTotal; /* 围护热损失/渗透热损失/总负荷 W */

void heatlossCalc();
#endif