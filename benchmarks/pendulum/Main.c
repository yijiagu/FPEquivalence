#include <stdio.h>
#include <math.h>
#include <mpfr.h>


int main(){
    double maxd = 0.0;
    double ulpd = 0.0;
    double result1 = 0.0;
    double result2 = 0.0;
    double max_input = 0.0;
    
    double tbegin = 0.1;
    double tend = 1;
    double wbegin = 0.1;
    double wend = 0.2;

    double t = tbegin;
    double w = wbegin;
    double max_t;
    int sampleNumbers = 10000;

    for(int i = 0; i < sampleNumbers; i ++){
        for(int j = 0; j < sampleNumbers; j ++){
            double t1 = t + 0.01*(w + 0.01/2*(-9.80665/2.0 * sin(t)));
            double t2 = t + 0.01*(w + (0.01/2*(-9.80665)/2.0) * sin(t));
            double epsilon = fabs(t1 - t2) / fabs(t1);
            if(epsilon > maxd){
                max_t = t;
                maxd = epsilon;
            }
            t = t + (tend - tbegin)/sampleNumbers;
        }
        t = tbegin;
        w = w + (wend - wbegin)/sampleNumbers;
    }

    printf("Max epsilon %.17f\n", maxd);
    printf("Max t %.17f\n", max_t);
}

