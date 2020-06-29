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
    double vend = 1.1;
    double epsbegin = 1;
    double epsend = 1.1;

    double x = vbegin;
    double eps =epsbegin;
    double max_x;
    int sampleNumbers = 10000;

    for(int i = 0; i < sampleNumbers; i ++){
        for(int j = 0; j < sampleNumbers; j ++){
            double t1 = ((((sin(eps) * ((1.0 - cos((x + x))) / 2.0)) / (cos(x) * cos(eps))) + (cos(x) * (sin(eps) / cos(eps)))) / (cos(x) * (1.0 - ((sin(eps) * sin(x)) / (cos(x) * cos(eps))))));
            double t2 = (((((sin(eps)  - sin(eps) * cos((x + x))) / 2.0) / (cos(x) * cos(eps))) + (cos(x) * (sin(eps) / cos(eps)))) / (cos(x) * (1.0 - ((sin(eps) * sin(x)) / (cos(x) * cos(eps))))));
            
            double epsilon = fabs(t1 - t2) / fabs(t1);
            if(epsilon > maxd){
                max_x = x;
                maxd = epsilon;
            }
            eps = eps + (epsend - epsbegin)/sampleNumbers;
        }
        eps = epsbegin;
        x = x + (vend - vbegin)/sampleNumbers;
        
    }

    printf("Max epsilon %.17f\n", maxd);
    printf("Max x %.17f\n", max_x);
}

