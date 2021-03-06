#include <stdio.h>
#include <math.h>
#include <mpfr.h>


int main(){
    double maxd = 0.0;
    double ulpd = 0.0;
    double result1 = 0.0;
    double result2 = 0.0;
    double max_input = 0.0;
    
    double vbegin = 2;
    double vend = 3;

    double x = vbegin;
    double max_x;
    int sampleNumbers = 1000000;

    for(int i = 0; i < sampleNumbers; i ++){
        double t1 = ((1.0 - m) * (((m / (v / 1.0)) - ((m * m) / v)) - 1.0));
        double t2 = ((1.0 - m) * (((m / (v)) - ((m * m) / v)) - 1.0));
        double epsilon = fabs(t1 - t2) / t1;
        if(epsilon > maxd){
            max_x = x;
            maxd = epsilon;
        }
        x = x + (vend - vbegin)/sampleNumbers;
    }

    printf("Max epsilon %.17f\n", maxd);
    printf("Max x %.17f\n", max_x);
}

