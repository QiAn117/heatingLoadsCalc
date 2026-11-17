#ifndef KCORRECT_H
#define KCORRECT_H

#include "common.h"

/* M3 传热系数修正计算模块 */
void kCorrect(void);

/* 按朝向代码取修正系数 alpha：N/S/E/W 取 g_wx 中的对应值，R（屋面）/U（地面）取 1.00。
 * 单独导出，供 M3 与 M9 报表共用，避免报表里用 kAdj/k 反推 alpha（kAdj 未算时为 0/0）。 */
double alphaForOrient(char orient);

#endif
