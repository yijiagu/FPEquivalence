#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double x){
    double r1 = (((((x * 0.022222222222222223) * x) + 0.3333333333333333) * x) + (pow(x, 5.0) * 0.0021164021164021165));
    separate();
    double r2 = (((log(exp(((x * 0.022222222222222223) * x))) + 0.3333333333333333) * x) + (pow(x, 5.0) * 0.0021164021164021165));
    return compFusion(r1, r2);
}

