#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double x){
    double r1 = ((x - sin(x)) / (x - tan(x)));
    separate();
    double r2 = ((x - sin(x)) / ((x - log(sqrt(exp(tan(x))))) - log(sqrt(exp(tan(x))))));
    return compFusion(r1, r2);
}

