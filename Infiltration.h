#ifndef Infiktration_H
#define Infiktration_H
#include "common.h"
#include <stdio.h>
extern struct Envelope g_face[MAX_FACE];
extern struct Weather  g_wx;
extern double tn;
extern double g_qLeak;
double leakLoss(double tn);

#endif