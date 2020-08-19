#include <stdio.h>
#include <stdlib.h>

/* print Fahrenheit-Celsius table */

int main(int argc, char **argv)
{

    int lower = 0;      /* lower limit of table */
    int upper = 300;    /* upper limit */
    int step = 20;      /* step size */

    if (argc == 2){
        lower = atoi(argv[1]);
        upper = lower;
        step = 1;
    }else if(argc == 4){
        lower = atoi(argv[1]);
        upper = atoi(argv[2]);
        step = atoi(argv[3]);
    }else{
        printf("Please use either one of the following formats:\n");
        printf("./a.out <fahrenheit>\n");
        printf("./a.out <lower limit> <upper limit> <step>\n");
	return 0;
    }

    int fahr;

    for (fahr = lower; fahr <= upper; fahr = fahr + step)
        printf("Fahrenheit: %3d, Celcius: %6.1f\n", fahr, (5.0/9.0)*(fahr-32));

    return 0;
}
