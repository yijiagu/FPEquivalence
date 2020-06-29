#include <stdio.h>
#include <math.h>

#define FUSION_TYPE 2


void separate();
double compFusion(double, double);

double demo_ic(double v){
      double p = 35000000.0;
      double a = 0.401;
      double b = 4.27e-05;
      double t = 300.0;
      double n = 1000.0;
      double k = 1.3806503e-23;
    
     double r1 = (((p + ((a * (n / v)) * (n / v))) * (v - (n * b))) - ((k * n) * t));
     separate();
     double r2 = (((p + ((a * n / v) * (n / v))) * (v - (n * b))) - ((k * n) * t));

     return compFusion(r1, r2);
}

