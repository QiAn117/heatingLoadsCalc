#ifndef INFILTRATION_H
#define INFILTRATION_H

#include "common.h"

/* 北/南窗渗风量 m3/h（= 每米缝隙渗风量 × 缝隙总长） */
double leakFlowN(void);
double leakFlowS(void);

/* 渗透总系数 C = 0.28·c·ρ·leak·(gapN+gapS)，单位 W/K（M6/M9 复用） */
double leakCoeff(void);

/* 纯计算：给定 tnRoom 返回渗透热损失 W，不打印、不写全局变量（供 M5/M6/M7 复用） */
double leakLoss(double tnRoom);

/* M5 冷风渗透热损失模块：计算并打印过程量，结果写入 g_qLeak */
void infiltrationCalc(double tnRoom);

#endif
