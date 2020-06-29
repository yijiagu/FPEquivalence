#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double m, double v){    
    double r1 = ((1.0 - m) * (((m / (v / 1.0)) - ((m * m) / v)) - 1.0));
    separate();
    double r2 = ((1.0 - m) * (((m / (v)) - ((m * m) / v)) - 1.0));
    return compFusion(r1, r2);
}

