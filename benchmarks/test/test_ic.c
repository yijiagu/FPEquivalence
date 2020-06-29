#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compSum(double, double);
double compDiff(double, double);
double compFusion(double, double);

double demo_ic(double x){
    
    double y1 = exp(x);
    double r1 = (y1 - 1)/x;

    separate();

    double y2 = exp(x);
    double r2 = (y2 - 1)/log(y2);

    return compFusion(r1, r2);
}

