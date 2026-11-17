#ifndef HEATLOSS_H
#define HEATLOSS_H

#include "common.h"

/* 围护结构总传热能力 S = Σ(kAdj·area)，单位 W/K（平衡迭代与报表复用） */
double envCoeff(void);

/* 纯计算：给定 tnRoom，按 q = kAdj·area·(tnRoom-tw) 填 g_face[].q，
 * 累加得 g_qEnv 并返回。不打印，供 M4/M7/M8/M9 复用。 */
double heatLossFace(double tnRoom);

/* M4 围护热损失逐面累加模块：计算并打印逐面热损失表与占比 */
void heatlossCalc(double tnRoom);

#endif
