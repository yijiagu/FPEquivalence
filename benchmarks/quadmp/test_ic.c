#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double a, double b, double c){
    double r1 = (((- b) - sqrt(((b * b) - (4.0 * (a * c))))) / (2.0 * a));
    separate();
    double r2 = (((- b) - sqrt(((b * b) - ((c * 4.0) * a)))) / (2.0 * a));
    return compFusion(r1, r2);
}

