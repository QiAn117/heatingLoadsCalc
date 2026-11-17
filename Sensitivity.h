#ifndef SENSITIVITY_H
#define SENSITIVITY_H

#include "common.h"

/* 表1：室温 14~24 C（步长 2 C）→ 围护/渗透/总负荷 */
void sensitivityTemps(FILE *fp);

/* 表2：供热能力 2000~4000 W（步长 500 W）→ 平衡室温 */
void sensitivityCapacity(FILE *fp);

/* M8 菜单入口：打印两张表（含标题）并复位设计工况结果 */
void sensitivity(void);

#endif
