#ifndef HEATLOSS_H
#define HEATLOSS_H

#include "common.h"
#include <stdio.h>

void   heatlossCalc(void);       /* M4：计算并打印逐面热损失表与占比 */
double heatLossFace(double tn);  /* 纯计算：给定 tn 填 g_face[].q 并累加 g_qEnv，返回 g_qEnv */

#endif
