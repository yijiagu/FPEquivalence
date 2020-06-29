#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double x){
    double r1 = ((1.0 - (log(exp(tan(x))) * tan(x))) / ((log(exp(tan(x))) * log(exp(tan(x)))) + 1.0));
    separate();
    double r2 = ((1.0 - (tan(x) * tan(x))) / ((log(exp(tan(x))) * log(exp(tan(x)))) + 1.0));
    return compFusion(r1, r2);
}

