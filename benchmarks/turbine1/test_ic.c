#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double w, double v, double r){
    double r1 =  ((3.0 - 4.5) + (((2.0 / r) / r) - (((1.0 / (1.0 - v)) / (1.0 / ((w * r) * (w * r)))) * ((3.0 - (v * 2.0)) * 0.125))));
    separate();
    double r2 = ((3.0 - 4.5) + (((2.0 / (r * r))) - (((1.0 / (1.0 - v)) / (1.0 / ((w * r) * (w * r)))) * ((3.0 - (v * 2.0)) * 0.125))));
    return compFusion(r1, r2);
}

