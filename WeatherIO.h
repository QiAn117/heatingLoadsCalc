#ifndef WEATHERIO_H
#define WEATHERIO_H

#include "common.h"

/* M2 室外气象参数文件读入模块
 * 返回 1 表示文件成功打开并完成读入（缺项时给警告并沿用默认值）；
 * 返回 0 表示文件打不开，此时上一份参数保持不变。 */
int loadWeather(void);

#endif
