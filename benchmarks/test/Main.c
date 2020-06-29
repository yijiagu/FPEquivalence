#include <stdio.h>
#include <math.h>
#include <mpfr.h>


int main(){
    double maxd = 0.0;
    double ulpd = 0.0;
    double result1 = 0.0;
    double result2 = 0.0;
    double max_input = 0.0;
    
    double vbegin = 0.00000000000001;
    double vend = 0.001;

    double v = vbegin;

    
    
    for(int i = 0; i < 10000000; i ++){
        
        double y = exp(v);
        
        double t1 = (exp(v) - 1)/v;
        double t2 = (exp(v) - 1)/log(y);

        double epsilon = fabs(t1 - t2) / t1;
        if(epsilon > maxd){
            maxd = epsilon;
        }
        
        v =  v + (vend - vbegin)/10000000;
    }

    printf("Max epsilon %.17f\n", maxd);
}

