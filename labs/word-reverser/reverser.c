#include <stdio.h>

#define IN   1   /* inside a word */
#define OUT  0   /* outside a word */

/* count lines, words, and characters in input */
void reverse(char word[1000], int length){
    char tmp;
    int i;
    for (i = 0;  i < length/2; i++) {
        tmp = word[i];
        word[i] = word[length - i - 1];
        word[length - i - 1] = tmp;
    }
    for (i = 0; i < length; i++){
        putchar(word[i]);
    }
    printf("\n");
}

int main()

{
    int c, i, length;
    char word[1000];

    while ((c = getchar()) != EOF) {

	    word[i++] = c;
	    if (c == '\n'){
            reverse(word, i - 1);
            i = 0;
        }

    }

    return 0;
}