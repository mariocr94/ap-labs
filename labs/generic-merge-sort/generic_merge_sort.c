#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINES 5000
char *lineptr[MAXLINES];

// int readlines(char *lineptr[], int nlines, char *f);
// void writelines(char *lineptr[], int nlines);

void myMergeSort(void *v[], int left, int right, int (*comp)(void *, void *));

void merge (void *v[], int left, int middle, int right, int (*comp)(void *, void *));

int numcmp(char *, char *);

int main(int argc, char *argv[])
{
    int nlines = 0;
    int numeric = 0;
    int fileN = 1;
    
    if(argc == 3 && strcmp(argv[1], "-n") == 0){
        fileN = 2;
        numeric = 1;
    }else if(argc == 2 && strcmp(argv[1], "-n") == 0){
        printf("Please use the correct notation: \n");
        printf("./generic_merge_sort.c -n <numbers file>\n");
        printf("./generic_merge_sortc. <strings file>\n");
        exit(0);
    }else if(argc < 2){
        printf("Please use the correct notation: \n");
        printf("./generic_merge_sort.c -n <numbers file>\n");
        printf("./generic_merge_sortc. <strings file>\n");
        exit(0);
    }
    
    /* readlines piece of code */
    char line[100] = {0};
    FILE *file = fopen(argv[fileN], "r");

    if(!file){
        printf("Couldn't open the file %s.\n", argv[fileN]);
        return -1;
    }

    while(fgets(line, 100, file)){
        lineptr[nlines] = (char*)malloc(strlen(line) + sizeof(char*));
        strcpy(lineptr[nlines],line);
        nlines++;
    }

    if(fclose(file)){
        printf("Couldn't close the file.\n");
    }

    // end of readline code

    if ( nlines>= 0){
        if(numeric){
            myMergeSort(&lineptr, 0, nlines-1,numcmp);
            for(int k = 0;k<nlines;k++){
                printf("%i: %s",k, lineptr[k]);
            }
            return 0;
        }else{
            myMergeSort(&lineptr, 0, nlines-1,strcmp);
            for(int k = 0;k<nlines;k++){
                printf("%s",lineptr[k]);
            }
            return 0;
        }
        
    }else{
        printf("input too big to sort\n");
        return 1;
    }
}

void myMergeSort(void *v[], int left, int right, int (*comp)(void *, void *)){
    if (left < right) 
    { 
        // Same as (l+r)/2, but avoids overflow for 
        // large l and h 
        int middle = left+(right-left)/2; 
        
        // Sort first and second halves 
        myMergeSort(v, left, middle, comp); 
        myMergeSort(v, middle+1, right, comp); 
  
        merge(v, left, middle, right, comp); 
    } 
}

void merge (void *v[], int left, int middle, int right, int (*comp)(void *, void *)){
    int i, j, k; 
    int n1 = middle - left + 1; 
    int n2 =  right - middle; 

    void *L[n1], *R[n2]; 

    for (i = 0; i < n1; i++){
        // strcpy(L[i], v[left+i]);
        L[i] = v[left + i];
    }

    for (j = 0; j < n2; j++) 
        R[j] = v[middle + 1+ j]; 
        // strcpy(R[j],v[middle + 1+ j]);

    i = 0; 
    j = 0; 
    k = left; 
    while (i < n1 && j < n2){ 
        if (comp(L[i],R[j])<=0) { 
            // strcpy(v[k],L[i]);
            v[k] = L[i]; 
            i++; 
        } 
        else
        { 
            // strcpy(v[k],R[j]);
            v[k] = R[j]; 
            j++; 
        } 
        
        k++; 
    } 

    while (i < n1) 
    { 
        // strcpy(v[k],L[i]);
        v[k] = L[i]; 
        i++; 
        k++; 
    } 

    while (j < n2) 
    { 
        // strcpy(v[k],R[j]);
        v[k] = R[j]; 
        j++; 
        k++; 
    } 
}



int numcmp(char *s1, char *s2){
    double v1, v2;

    v1 = atof(s1);
    v2 = atof(s2);
    if (v1 < v2)
        return -1;
    else if(v1 > v2)
        return 1;
    else
        return 0;
}

void swap(void *v[], int i, int j){
    void *temp;
    printf("i: %i value: %s, j: %i value: %s", i, v[i], j, v[j]);
    temp = v[i];
    v[i] = v[j];
    v[j] = temp;
}

int readlines(char *lineptr[], int nlines, char *f){
    char line[100] = {0};
    int line_count = 0;
    char **v;

    FILE *file = fopen(f, "r");

    if(!file){
        printf("Couldn't open the file.\n");
        return -1;
    }

    
    // while(fgets(line, 100, file)){
        fgets(line, 100, file);
        v = &lineptr[0];
        *v = line;
        printf("%s", lineptr[0]);
        line_count++;
        

        fgets(line, 100, file);
        v = &lineptr[1];
        *v = line;
        printf("%s", lineptr[1]);
        line_count++;
    // }
    if(fclose(file)){
        printf("Couldn't close the file.\n");
    }
    return line_count;
}