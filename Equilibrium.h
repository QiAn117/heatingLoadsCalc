#ifndef EQUILIBRIUM_H
#define EQUILIBRIUM_H

#include "common.h"
#include <stdio.h>

void   equilibrium(void);                     /* M6 菜单入口：打印迭代过程 */
double iterEquilibrium(double qsup, int verbose); /* 纯迭代，供 M8/M9 批量复用 */

#endif
