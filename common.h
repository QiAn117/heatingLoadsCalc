#ifndef COMMON_H
#define COMMON_H

/* ==================================================================
 *  房间冬季采暖负荷计算系统 —— 公共定义
 *  集中定义：结构体、全局变量、校验区间、迭代参数、文件名，
 *            以及少量跨模块的通用小工具（BOM、字符串、对齐输出）。
 *
 *  变量命名约定：
 *    g_xxx    全局变量（见下方 extern 声明）
 *    xxx(void) 无参函数一律写 (void)，避免"参数未声明"的老式写法
 *    tnRoom   函数参数里的"当前房间温度"，与全局 g_tnDesign/g_tn 区分
 * ================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------- 全局常量 ------------------------- */

#define MAX_FACE 20              /* 最大围护面数，超出部分读入时忽略并告警 */

#define C_AIR   1.0              /* 空气比热容 c   kJ/(kg*C) */
#define RHO_AIR 1.29             /* 空气密度   rho kg/m3     */

#define TN_DESIGN_DEFAULT 18.0   /* 设计室温默认值 C */

/* M6 不动点迭代参数 */
#define ITER_MAX  100            /* 迭代次数上限 */
#define ITER_EPS  0.01           /* 收敛判据 |tn_new - tn_old| < 0.01 C */
#define ITER_FAIL (-1e9)         /* 迭代失败时的返回值 */
#define ITER_OK(t) ((t) > -1e8)  /* 判断迭代结果是否有效 */

/* 数据合法性区间（M1 围护表 / M2 气象参数校验用） */
#define AREA_MIN  1.0            /* 单个围护面面积 m2  */
#define AREA_MAX  50.0
#define K_MIN     0.2            /* 传热系数 W/(m2K)   */
#define K_MAX     3.5
#define LEAK_MIN  0.5            /* 每米缝隙渗风量 m3/(m*h) */
#define LEAK_MAX  2.0
#define GAP_MIN   0.0            /* 缝隙总长 m */
#define GAP_MAX   20.0
#define QSUP_MIN  1000.0         /* 散热器供热能力 W */
#define QSUP_MAX  6000.0
#define ALPHA_MIN 0.5            /* 朝向修正系数 */
#define ALPHA_MAX 1.5
#define TN_MIN    (-50.0)        /* 交互输入的室温合理范围 C */
#define TN_MAX    50.0

/* M8 敏感性实验区间（M9 报表复用，保证两处表格完全一致） */
#define SENS_TN_BEGIN 14.0       /* 室温实验起点 C */
#define SENS_TN_END   24.0       /* 室温实验终点 C */
#define SENS_TN_STEP  2.0        /* 室温步长 C */
#define SENS_Q_BEGIN  2000.0     /* 供热能力实验起点 W */
#define SENS_Q_END    4000.0     /* 供热能力实验终点 W */
#define SENS_Q_STEP   500.0      /* 供热能力步长 W */

/* 单位面积采暖负荷指标经验范围 W/m2 */
#define IDX_LO 50.0
#define IDX_HI 80.0

/* 数据文件名（集中定义，避免各模块各写一份字符串） */
#define ENVELOPE_FILE "envelope.txt"
#define WEATHER_FILE  "weather.txt"
#define REPORT_FILE   "heat_load_report.txt"

/* ------------------------- 数据结构 ------------------------- */

/* 单个围护面 */
struct Envelope {
    char   name[16];     /* 面名称，UTF-8 下约 5 个汉字 */
    char   orient;       /* 朝向代码 N/S/E/W/R/U */
    double area;         /* 面积 m2，取值 1~50 */
    double k;            /* 原始传热系数 W/(m2K)，0.2~3.5 */
    double kAdj;         /* 修正后传热系数 = k*alpha（M3 写入） */
    double q;            /* 本面传热热损失 W（M4 写入） */
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

/* ------------------------- 全局变量 ------------------------- */

extern struct Envelope g_face[MAX_FACE];
extern struct Weather  g_wx;
extern int    g_faceCnt;        /* 围护面数（含无效行） */

extern double g_tnDesign;       /* 设计室温 C（设计工况用，交互设定） */
extern double g_tn;             /* 平衡室温 C（M6 迭代求出） */
extern int    g_iterCnt;        /* M6 实际迭代轮数，供报表引用 */

extern double g_qEnv;           /* 围护传热热损失 W */
extern double g_qLeak;          /* 冷风渗透热损失 W */
extern double g_qTotal;         /* 总负荷 W */

/* ------------------------- 通用小工具 ------------------------- */

/* 跳过文件开头的 UTF-8 BOM（EF BB BF）；没有 BOM 时把读写位置复位到文件开头。
 * M1、M2 共用：学生用 Windows 记事本另存的文件常带 BOM，
 * 若不跳过，第一行的关键字/表头会多出三个不可见字节而解析失败。 */
static inline void skipUtf8Bom(FILE *fp)
{
    int b1 = fgetc(fp);
    if (b1 == 0xEF) {
        int b2 = fgetc(fp);
        int b3 = fgetc(fp);
        if (b2 == 0xBB && b3 == 0xBF) return;   /* 是 BOM，已跳过 */
    }
    rewind(fp);                                 /* 不是 BOM：回到开头 */
}

/* 原地去掉字符串首尾的空白字符（空格、制表符、回车、换行），返回同一指针 */
static inline char *trim(char *s)
{
    size_t n;
    char  *p = s;

    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    n = strlen(p);
    while (n > 0 && (p[n - 1] == ' ' || p[n - 1] == '\t' ||
                     p[n - 1] == '\r' || p[n - 1] == '\n'))
        p[--n] = '\0';
    if (p != s) memmove(s, p, n + 1);
    return s;
}

/* 把整个字符串解析为 double：成功返回 1；空串、非数字或带多余字符返回 0。
 * 相比 atof()，这样可以发现 "abc"、"1.2x" 这类录入错误，而不是静默当成 0。 */
static inline int parseDouble(const char *s, double *out)
{
    char *end = NULL;
    double v;

    while (*s == ' ' || *s == '\t') s++;
    if (*s == '\0') return 0;
    v = strtod(s, &end);
    if (end == s) return 0;
    while (*end == ' ' || *end == '\t' || *end == '\r' || *end == '\n') end++;
    if (*end != '\0') return 0;
    *out = v;
    return 1;
}

/* 估算 UTF-8 字符串在中文控制台中的显示宽度（全角字符按 2 列计算）。
 * 用 printf("%-8s") 直接对齐中文会错位，因为 %-8s 按"字节数"补空格，
 * 而 3 个汉字占 9 字节、只占 6 列。 */
static inline int textWidth(const char *s)
{
    int w = 0;
    const unsigned char *p = (const unsigned char *)s;

    while (*p != '\0') {
        if (*p < 0x80) {
            w += 1;
            p += 1;
        } else if ((*p & 0xE0) == 0xC0 && p[1] != '\0') {
            w += 1;                             /* 2 字节字符（拉丁扩展等） */
            p += 2;
        } else if ((*p & 0xF0) == 0xE0 && p[1] != '\0' && p[2] != '\0') {
            w += 2;                             /* 3 字节字符（汉字等全角） */
            p += 3;
        } else if (p[1] != '\0' && p[2] != '\0' && p[3] != '\0') {
            w += 2;                             /* 4 字节字符（emoji 等） */
            p += 4;
        } else {
            break;                              /* 残缺的 UTF-8 序列，停止 */
        }
    }
    return w;
}

/* 按显示宽度把字符串写到 fp，再补空格到 width 列（各模块表格统一用它对齐名称列） */
static inline void fprintPadded(FILE *fp, const char *s, int width)
{
    int i;
    fputs(s, fp);
    for (i = textWidth(s); i < width; i++) fputc(' ', fp);
}

/* 同 fprintPadded，输出到屏幕 */
static inline void printPadded(const char *s, int width)
{
    fprintPadded(stdout, s, width);
}

/* 表头行：每列占 8 列（2 个空格 + 6 列文本），最后一列不补尾空格。
 * 与数据行的 printf("  %6.2f") 列宽一致，中英文混排也不会错位。 */
static inline void fprintHeader(FILE *fp, const char *const *cols, int n)
{
    int i;
    for (i = 0; i < n; i++) {
        fputc(' ', fp);
        fputc(' ', fp);
        if (i < n - 1) fprintPadded(fp, cols[i], 6);
        else           fputs(cols[i], fp);
    }
    fputc('\n', fp);
}

/* 同 fprintHeader，输出到屏幕 */
static inline void printHeader(const char *const *cols, int n)
{
    fprintHeader(stdout, cols, n);
}

#endif
