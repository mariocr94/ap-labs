#include <stdio.h>
#include <stdlib.h>

/* month_day function's prototype*/
void month_day(int year, int yearday, int *pmonth, int *pday);

static char daytab[2][12] = { {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31} };
static char *months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

int main(int argc, char **argv) {
    
    if(argc != 3){
        printf("Please use the following format:\n");
        printf("./month_day <year> <yearday>\n");
        return 0;
    }

    int year = atoi(argv[1]);
    int yearday = atoi(argv[2]);
    int pmonth = 0;
    int pday = 0;

    int leap;
    leap = year%4 == 0 && year%100 != 0 || year%400 == 0;

    if(leap ==1 && yearday > 366){
        printf("The year %i only has 366 days, please select a day between 0-366 inclusive.\n", year);
        return 0;
    }else if(leap == 0 && yearday > 365){
         printf("The year %i only has 365 days, please select a day between 0-365 inclusive.\n", year);
         return 0;
    }

    month_day(year,yearday,&pmonth,&pday);
    printf("%s %i, %i\n", months[pmonth],pday,year);
    return 0;
}

void month_day(int year, int yearday, int *pmonth, int *pday){

    int i, leap;
    leap = year%4 == 0 && year%100 != 0 || year%400 == 0;

    for(i = 0; yearday>0; i++){
        yearday -= daytab[leap][i];
    }
    i-=1;
    *pmonth = i;
    *pday = yearday + daytab[leap][i];

}
