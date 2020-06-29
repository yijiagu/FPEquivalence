#include <stdio.h>
#include <math.h>
#include <mpfr.h>


int main(){
    double maxd = 0.0;
    double ulpd = 0.0;
    double result1 = 0.0;
    double result2 = 0.0;
    double max_input = 0.0;
    
    double vbegin = 1;
    double vend = 1.5;

    double x = vbegin;
    double max_x;
    int sampleNumbers = 1000000;

    for(int i = 0; i < sampleNumbers; i ++){
        double t1 = ((x - sin(x)) / (x - tan(x)));
        double t2 = ((x - sin(x)) / ((x - log(sqrt(exp(tan(x))))) - log(sqrt(exp(tan(x))))));
        
        double epsilon = fabs(t1 - t2) / fabs(t1);
        if(epsilon > maxd){
            max_x = x;
            maxd = epsilon;
        }
        x = x + (vend - vbegin)/sampleNumbers;
    }

    printf("Max epsilon %.17f\n", maxd);
    printf("Max x %.17f\n", max_x);
}

