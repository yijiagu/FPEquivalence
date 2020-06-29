#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double x){    
    double r1 = ((0.954929658551372 * x) - (0.12900613773279798 * ((x * x) * x)));
    separate();
    double r2 = ((0.954929658551372 - ((x * 0.12900613773279798) * x)) * x);
    return compFusion(r1, r2);
}

