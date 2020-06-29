#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double w, double v, double r){
    double r1 =  (((3.0 - (2.0 / r / r)) - (((0.125 * (1.0 + (2.0 * v))) * (((w * w) * r) * r)) / (1.0 - v))) - 0.5);
    separate();
    double r2 = (((3.0 - (2.0 / (r * r))) - (((0.125 + (0.25 * v)) * (((w * w) * r) * r)) / (1.0 - v))) - 0.5);
    return compFusion(r1, r2);
}

