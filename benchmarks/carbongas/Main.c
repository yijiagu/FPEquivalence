#include <stdio.h>
#include <math.h>
#include <mpfr.h>


int main(){
    double maxd = 0.0;
    double ulpd = 0.0;
    double result1 = 0.0;
    double result2 = 0.0;
    double max_input = 0.0;
    
    double vbegin = 0.05;
    double vend = 10;
    
    double p = 35000000.0;
    double a = 0.401;
    double b = 4.27e-05;
    double t = 300.0;
    double n = 1000.0;
    double k = 1.3806503e-23;

    double v = vbegin;
    double max_x;
    int sampleNumbers = 1000000;

    for(int i = 0; i < sampleNumbers; i ++){
        double t1 = (((p + ((a * (n / v)) * (n / v))) * (v - (n * b))) - ((k * n) * t));
        double t2 = (((p + ((a * n / v) * (n / v))) * (v - (n * b))) - ((k * n) * t));
        double epsilon = fabs(t1 - t2) / t1;
        if(epsilon > maxd){
            max_x = v;
            maxd = epsilon;
        }
        v = v + (vend - vbegin)/sampleNumbers;
    }

    printf("Max epsilon %.17f\n", maxd);
    printf("Max x %.17f\n", max_x);
}

