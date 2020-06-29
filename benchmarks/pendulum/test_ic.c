#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double w, double t){
    double r1 = t + 0.01*(w + 0.01/2*(-9.80665/2.0 * sin(t)));
    separate();
    double r2 = t + 0.01*(w + (0.01/2*(-9.80665)/2.0) * sin(t));
    return compFusion(r1, r2);
}

