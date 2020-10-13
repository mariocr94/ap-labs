#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <inttypes.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include "logger.h"

#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66

#define ENCODING 0
#define DECODING 1

static const unsigned char d[] = {
    66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66
};

int status;
off_t size;
size_t progress;

int encode(char* fName);
int decode(char* fName);
int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize);
int base64decode (char *in, size_t inLen, unsigned char *out, size_t *outLen);
void showProgress();


int main(int argc, char **argv){
    initLogger("stdout");

    if(argc != 3){
        warnf("Please use the correct format: \n ./base64 < --encode / --decode > <file.txt>\n");
        return 0;
    }

    if(strcmp(argv[1], "--encode") == 0){
        signal(SIGINT, showProgress);
        signal(SIGUSR1, showProgress);
        infof("Starting to encode file: %s", argv[2]);
        status = ENCODING;
        encode(argv[2]);
    }else if(strcmp(argv[1], "--decode") == 0){
        signal(SIGINT, showProgress);
        signal(SIGUSR1, showProgress);
        infof("Starting to decode file: %s", argv[2]);
        status = DECODING;
        decode(argv[2]);
    }else{
        warnf("Please use the correct format: \n ./base64 < --encode / --decode > <file.txt>\n");
        return 0;
    }
    return 0;
}

int encode(char* fName){
    FILE *file = fopen(fName, "r");
    FILE *encFile;
    char *fileBuf, *encBuf;
    off_t bufsize;
    int caso;
    size_t encSize; 

    if(!file){
        errorf("Could not open file: %s.\n", fName);
        return 0;
    }

        /* Go to the end of the file. */
    if (fseek(file, 0L, SEEK_END) == 0) {
        /* Get the size of the file. */
        bufsize = ftell(file);
        encSize = 4 * ((bufsize + 2) / 3.0) + 3;
        if (bufsize == -1) { /* Error */ 
            errorf("Size too big");
            return 0;
        }
        size = bufsize;
        /* Allocate our buffer to that size. */
        fileBuf = (char *)malloc(sizeof(char) * (bufsize + 1));
        if (fileBuf == NULL) {
            panicf("Cannot allocate memory for destination buffer\n");
            fclose(file);
            return 0;
        }
        encBuf = (char *)malloc(sizeof(char) * (encSize + 1));

        /* Go back to the start of the file. */
        if (fseek(file, 0L, SEEK_SET) != 0) { /* Error */ }

        /* Read the entire file into memory. */
        
        size_t newLen = fread(fileBuf, sizeof(char), bufsize, file);

        if ( ferror( file ) != 0 ) {
            fputs("Error reading file", stderr);
        } else {
            fileBuf[newLen++] = '\0'; /* Just to be safe. */
        }
    }
    
    fclose(file);
    char *token = strtok(fName, ".");
    strcat(token,"-encoded.txt");
    if ((caso=base64encode(fileBuf, bufsize, encBuf, encSize)) != 0){
        panicf("Unable to encode file, error: %i\n", caso);
        free(fileBuf);
        free(encBuf);
        return 0;
    }

    if ((encFile = fopen(token, "w")) == NULL){
        panicf("Unable to create destination file\n");
        free(fileBuf);
        free(encBuf);
        return 0;
    }

    size_t newLen;
    if(newLen = fwrite(encBuf, sizeof(char), bufsize, encFile) == 0){
        panicf("Unable to write to destination file\n");
        fclose(encFile);
        free(fileBuf);
        free(encBuf);
        return 0;
    }

    infof("Succesfully encoded the file %s into %s.", fName, token);

    fclose(encFile);
    free(fileBuf);
    free(encBuf);
}

int decode(char* fName){
        FILE *file = fopen(fName, "r");
    FILE *encFile;
    char *fileBuf, *encBuf;
    size_t bufsize;
    int caso;
    size_t encSize; 

    if(!file){
        errorf("Could not open file: %s.\n", fName);
        return 0;
    }

        /* Go to the end of the file. */
    if (fseek(file, 0L, SEEK_END) == 0) {
        /* Get the size of the file. */
        bufsize = ftell(file);
        encSize = 4 * ((bufsize + 2) / 3.0) + 3;
        if (bufsize == -1) { /* Error */ 
            errorf("Size too big");
            return 0;
        }
        size = bufsize;
        /* Allocate our buffer to that size. */
        fileBuf = (char *)malloc(sizeof(char) * (bufsize + 1));
        encBuf = (char *)malloc(sizeof(char) * (encSize + 1));
        /* Go back to the start of the file. */
        if (fseek(file, 0L, SEEK_SET) != 0) { /* Error */ }

        /* Read the entire file into memory. */
        size_t newLen = fread(fileBuf, sizeof(char), bufsize, file);
        if ( ferror( file ) != 0 ) {
            fputs("Error reading file", stderr);
        } else {
            fileBuf[newLen++] = '\0'; /* Just to be safe. */
        }
    }

    fclose(file);
    char *token = strtok(fName, ".");
    strcat(token,"-decoded.txt");
    if ((caso=base64decode(fileBuf, bufsize, (unsigned char *)encBuf, &encSize)) != 0){
        panicf("Unable to decode file, error: %i\n", caso);
        free(fileBuf);
        free(encBuf);
        return 0;
    }

    if ((encFile = fopen(token, "w")) == NULL){
        panicf("Unable to create destination file\n");
        free(fileBuf);
        free(encBuf);
        return 0;
    }

    size_t newLen;
    if(newLen = fwrite(encBuf, sizeof(char), bufsize, encFile) == 0){
        panicf("Unable to write to destination file\n");
        fclose(encFile);
        free(fileBuf);
        free(encBuf);
        return 0;
    }

    infof("Succesfully encoded the file %s into %s.", fName, token);

    fclose(encFile);
    free(fileBuf);
    free(encBuf);
}

int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize)
{
   const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   const uint8_t *data = (const uint8_t *)data_buf;
   size_t resultIndex = 0;
   size_t x;
   uint32_t n = 0;
   int padCount = dataLength % 3;
   uint8_t n0, n1, n2, n3;
    progress = 0;
   /* increment over the length of the string, three characters at a time */
   for (x = 0; x < dataLength; x += 3) 
   {
       progress++;
      /* these three 8-bit (ASCII) characters become one 24-bit number */
      n = ((uint32_t)data[x]) << 16; //parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0
      
      if((x+1) < dataLength)
         n += ((uint32_t)data[x+1]) << 8;//parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0
      
      if((x+2) < dataLength)
         n += data[x+2];

      /* this 24-bit number gets separated into four 6-bit numbers */
      n0 = (uint8_t)(n >> 18) & 63;
      n1 = (uint8_t)(n >> 12) & 63;
      n2 = (uint8_t)(n >> 6) & 63;
      n3 = (uint8_t)n & 63;
            
      /*
       * if we have one byte available, then its encoding is spread
       * out over two characters
       */
      if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
      result[resultIndex++] = base64chars[n0];
      if(resultIndex >= resultSize) return 2;   /* indicate failure: buffer too small */
      result[resultIndex++] = base64chars[n1];

      /*
       * if we have only two bytes available, then their encoding is
       * spread out over three chars
       */
      if((x+1) < dataLength)
      {
         if(resultIndex >= resultSize) return 3;   /* indicate failure: buffer too small */
         result[resultIndex++] = base64chars[n2];
      }

      /*
       * if we have all three bytes available, then their encoding is spread
       * out over four characters
       */
      if((x+2) < dataLength)
      {
         if(resultIndex >= resultSize) return 4;   /* indicate failure: buffer too small */
         result[resultIndex++] = base64chars[n3];
      }
   }

   /*
    * create and add padding that is required if we did not have a multiple of 3
    * number of characters available
    */
   if (padCount > 0) 
   { 
      for (; padCount < 3; padCount++) 
      { 
         if(resultIndex >= resultSize) return 5;   /* indicate failure: buffer too small */
         result[resultIndex++] = '=';
      } 
   }
   if(resultIndex >= resultSize) return 6;   /* indicate failure: buffer too small */
   result[resultIndex] = 0;
   return 0;   /* indicate success */
}

int base64decode (char *in, size_t inLen, unsigned char *out, size_t *outLen) { 
    char *end = in + inLen;
    char iter = 0;
    uint32_t buf = 0;
    size_t len = 0;
    progress = 0;
    while (in < end) {
        progress++;
        unsigned char c = d[*in++];
        
        switch (c) {
        case WHITESPACE: continue;   /* skip whitespace */
        case INVALID:    return 1;   /* invalid input, return error */
        case EQUALS:                 /* pad character, end of data */
            in = end;
            continue;
        default:
            buf = buf << 6 | c;
            iter++; // increment the number of iteration
            /* If the buffer is full, split it into bytes */
            if (iter == 4) {
                if ((len += 3) > *outLen) return 1; /* buffer overflow */
                *(out++) = (buf >> 16) & 255;
                *(out++) = (buf >> 8) & 255;
                *(out++) = buf & 255;
                buf = 0; iter = 0;

            }   
        }
    }
   
    if (iter == 3) {
        if ((len += 2) > *outLen) return 1; /* buffer overflow */
        *(out++) = (buf >> 10) & 255;
        *(out++) = (buf >> 2) & 255;
    }
    else if (iter == 2) {
        if (++len > *outLen) return 1; /* buffer overflow */
        *(out++) = (buf >> 4) & 255;
    }

    *outLen = len; /* modify to reflect the actual output size */
    return 0;
}

void showProgress(){
    infof("%s file\tProgress: %f \n",(status==ENCODING)?"Encoding":"Decoding", progress / size * 100.0f);
}
