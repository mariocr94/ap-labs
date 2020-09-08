#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int mystrlen(char *str){
    int charcount = 0;
    int i = 0;
    for(i = 0; str[i]; i++){
        if(str[i] != '\0')
            charcount++;
    }
    return charcount;
}

char *mystradd(char *origin, char *addition){
    char *newStr = (char *) malloc(mystrlen(origin) + mystrlen(addition) + 1);
    strcpy(newStr, origin);
    strcat(newStr, addition);
    return newStr;
}

int mystrfind(char *origin, char *substr){
    int len = mystrlen(origin);
    int sublen = mystrlen(substr);
    int i, j;

    for(i=0; i<len;i++){
        if(origin[i] == substr[0]){
            for (j=1;j<sublen;j++){
                if (origin[i+j] != substr[j])
                    break;
            }
            if(j == sublen)
                return i;
        }
    }
    return -1;
}
