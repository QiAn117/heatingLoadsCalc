#ifndef REPORT_H
#define REPORT_H

#include "common.h"

/* M9 负荷报表输出模块：把参数、逐面修正与热损失、渗透过程、设计工况汇总、
 * 热平衡迭代结果与两张敏感性实验表写入 heat_load_report.txt */
void writeReport(void);

#endif
