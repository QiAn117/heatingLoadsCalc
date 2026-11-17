#ifndef SUMLOAD_H
#define SUMLOAD_H

#include "common.h"

/* 房间面积 m2：优先取地面(U)面积，其次屋面(R)，都没有时退化为各面面积之和 */
double roomArea(void);

/* 单位面积负荷指标 W/m2；房间面积无法确定时返回 0 */
double loadIndicator(double qTotal);

/* 重新计算设计工况的 g_qEnv / g_qLeak / g_qTotal（不打印，供 M8/M9 收尾复位用） */
void refreshDesignLoad(void);

/* M7 采暖负荷汇总模块：用设计室温 g_tnDesign 调 M4/M5，汇总总负荷与指标，返回 g_qTotal */
double sumLoad(void);

#endif
