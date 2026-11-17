#ifndef ENVELOPEIO_H
#define ENVELOPEIO_H

#include "common.h"

/* M1 围护结构参数文件读入模块
 * 返回 1 表示文件成功打开并完成读入（允许存在无效行）；
 * 返回 0 表示文件打不开或为空，此时上一份数据保持不变。 */
int loadEnvelope(void);

#endif
