#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double x, double eps){
    double r1 = ((((sin(eps) * ((1.0 - cos((x + x))) / 2.0)) / (cos(x) * cos(eps))) + (cos(x) * (sin(eps) / cos(eps)))) / (cos(x) * (1.0 - ((sin(eps) * sin(x)) / (cos(x) * cos(eps))))));
    separate();
    double r2 = (((((sin(eps)  - sin(eps) * cos((x + x))) / 2.0) / (cos(x) * cos(eps))) + (cos(x) * (sin(eps) / cos(eps)))) / (cos(x) * (1.0 - ((sin(eps) * sin(x)) / (cos(x) * cos(eps))))));
    return compFusion(r1, r2);
}

