#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double r, double w, double v){
    double x = 1.2;
    double y = 2.5;
    double z = -3;

    double r1 = x + r + z + (y + v + w);
    separate();
    double r2 = x + r + z + (y + (v + w));
    return compFusion(r1, r2);

}

