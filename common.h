#ifndef COMMON_H
#define COMMON_H

#define MAX_FACE 20      /* 最大围护面数 */

/* 单个围护面 */
struct Envelope {
    char   name[16];     /* 面名称，≤15 字符 */
    char   orient;       /* 朝向代码 N/S/E/W/R/U */
    double area;         /* 面积 m2，取值 1~50 */
    double k;            /* 原始传热系数 W/(m2K)，0.2~3.5 */
    double kAdj;         /* 修正后传热系数 = k*alpha */
    double q;            /* 本面传热热损失 W */
    int    valid;        /* 数据有效性标志 1/0 */
};

/* 气象与系统参数 */
struct Weather {
    double tw;           /* 室外计算温度 C，哈尔滨约 -24.2，须<0 */
    double alphaN;       /* 北向修正 1.10 */
    double alphaEW;      /* 东西向修正 1.00 */
    double alphaS;       /* 南向修正 0.85 */
    double leak;         /* 每米缝隙渗风量 m3/(m*h)，0.5~2.0 */
    double gapN, gapS;   /* 北/南窗缝隙总长 m，0~20 */
    double qsup;         /* 散热器供热能力 W，取值 1000~6000 */
};
#endif