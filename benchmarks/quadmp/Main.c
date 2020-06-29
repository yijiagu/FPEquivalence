#include <stdio.h>
#include <math.h>
#include <mpfr.h>


int main(){
    double maxd = 0.0;
    double ulpd = 0.0;
    double result1 = 0.0;
    double result2 = 0.0;
    double max_input = 0.0;
    
    double rbegin = 0.0000000001;
    double rend = 0.2;
    double wbegin = 2;
    double wend = 3;
    double vbegin = 0.000000000000000001;
    double vend = 0.2;

    double r = rbegin;
    double w = wbegin;
    double v = vbegin;
    double max_v;
    int sampleNumbers = 1000;

    for(int i = 0; i < sampleNumbers; i ++){
        for(int j = 0; j < sampleNumbers; j ++){
            for(int k = 0; k < sampleNumbers; k ++){
                double t1 =  (((- w) - sqrt(((w * w) - (4.0 * (v * r))))) / (2.0 * v));
                double t2 = (((- w) - sqrt(((w * w) - ((r * 4.0) * v)))) / (2.0 * v));
                double epsilon = fabs(t1 - t2) / fabs(t1);
                if(epsilon > maxd){
                    max_v = v;
                    maxd = epsilon;
                }
                v = v + (vend - vbegin)/sampleNumbers;
            }
            v = vbegin;
            r = r + (rend - rbegin)/sampleNumbers;
        }
        r = rbegin;
        v = vbegin;
        w = w + (wend - wbegin)/sampleNumbers;
    }

    printf("Max epsilon %.17f\n", maxd);
    printf("Max t %.17f\n", max_v);
}

