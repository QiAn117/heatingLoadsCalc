#ifndef EQUILIBRIUM_H
#define EQUILIBRIUM_H

#include "common.h"

/* M6 菜单入口：按 g_wx.qsup 迭代求平衡室温，打印迭代过程并写入 g_tn */
void equilibrium(void);

/* 纯迭代：给定散热器供热能力 qsup，返回平衡室温 C；不收敛返回 ITER_FAIL。
 * verbose=1 时打印每轮迭代过程，供 M6 使用；报表/敏感性实验传 0。 */
double iterEquilibrium(double qsup, int verbose);

/* 写出"供热能力是否足够"的结论：M6 传 stdout，M9 传报表文件句柄 */
void printCapacityNote(FILE *fp, double qsup, double teq);

#endif
