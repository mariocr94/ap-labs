#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <fcntl.h> 

#define REMOVED 0
#define INSTALLED 1
#define UPDATED 2

#define FIND_ACTION 10
#define GET_ACTION 11
#define GET_DATE 12
#define GET_NAME 13
#define FIND_NEXT_LINE 14

// Package struct to keep track of each package.
struct Package {
    char name[50];
    char idate[17];
    char lupdate[17];
    int updates;
    char rdate[17];
    int status;
};
// Hashtable struct to have track of the packages being read.
struct Hashtable{
    int size;
    int nelements;
    struct Package array[1000];
};

#define REPORT_FILE "packages_report.txt"

void analizeLog(char *logFile, char *report);
bool isAction(char c1, char c2, char c3);
bool inHashTable(struct Hashtable *ht, char name[]);
void addToHashtable(struct Hashtable *ht,struct Package *p);;
struct Package *get(struct Hashtable *ht, char name[]);
void makeReport(char *rBuf, int iPacks, int rPacks, int uPacks, int cInstalled, struct Hashtable *ht);
void htToString(char report[], struct Hashtable *ht);
void pToString(char report[], struct Package *p);

int main(int argc, char **argv) {

    if (argc < 5) {
	printf("Please use the correct format:\n ./pacman-analizer.o  -input <pacman log.txt> -report <reportfile.txt>\n");
	return 1;
    }

    analizeLog(argv[2], argv[4]);

    return 0;
}

void analizeLog(char *logFile, char *report) {
    printf("Generating Report from: [%s] log file\n", logFile);

    struct Hashtable ht = {1000,0};
    int iPacks = 0;
    int uPacks = 0;
    int rPacks = 0;
    int cInstalled = 0;

    int fd = open(logFile, O_RDONLY);
    if (fd == -1)
    {
        printf("Failed to open the file, make sure you are selecting a file that exists.\n");
        return;
    }
    //Puts pointer at end of file.
    int size = lseek(fd, sizeof(char), SEEK_END);

    //Close file and open again to reset pointer.
    close(fd);
    fd = open(logFile, O_RDONLY);
    if (fd == -1)
    {
        printf("Failed to open the file 2.\n");
        return;
    }
    //Creates buffer with the size of the file and gets all the info.
    char buf[size];
    read(fd, buf, size);
    close(fd);
    //Adds null character at the end of buf.
    buf[size-1] = '\0';

    int i = 0;
    int j;
    int state = GET_DATE;
    char name[50];
    char date[17];
    char action[10]; //If installed, removed or upgraded
    bool validLine = false;
    
    //Starts cicle to go through every char of buf and start working
    while(i<size){
        //Switch case for each of the actions
        switch (state){
            case GET_DATE:
                if(buf[i] == '['){
                    //move buf to next value
                    i++;
                    j = 0;
                    //while to get the date between []
                    while (buf[i] != ']'){
                        date[j] = buf[i];
                        j++;
                        i++;
                    }
                    //Add null character to end of date.
                    date[j] = '\0';
                    i += 2; //Skip to next available character, since buf[i+1] is space.
                    state = FIND_ACTION;
                }else{
                    state = FIND_ACTION;
                }
                break;

            case FIND_ACTION:
                //go through [PACMAN] | [ALPM] | etc... and get the action.
                while(buf[i] != ' '){
                    i++;
                }
                i++;
                state = GET_ACTION;
                break;

            case GET_ACTION:
                j = 0;
                //Check if the next word is installed, removed or upgraded
                if(isAction(buf[i], buf[i+1], buf[i+2])){
                    while(buf[i] != ' '){
                        action[j] = buf[i];
                        i++;
                        j++;
                    }
                    action[j] = '\0';
                    i++;
                    validLine = true;
                    state = GET_NAME;
                }else{
                    // If it isnt a valid name, go to the next line.
                    state = FIND_NEXT_LINE;
                }
                break;

            case GET_NAME:
                j=0;
                // Check for the name.
                while(buf[i] != ' '){
                    name[j] = buf[i];
                    i++;
                    j++;
                }
                name[j] = '\0';
                i++;
                state = FIND_NEXT_LINE;
                break;

            case FIND_NEXT_LINE:
                //Looks for the next line break or null character.
                while (!(buf[i] == '\n' || buf[i] == '\0')){
                    i++;
                }
                i++;
                //Checks to see if the line was a valid action.
                if(validLine){
                    //Checks to see if the package is in the hash table
                    if(!inHashTable(&ht, name)){
                        //creates new package and changes the name and date.
                        struct Package p = {"", "", "", 0, "-", INSTALLED};
                        strcpy(p.name, name);
                        strcpy(p.idate, date);
                        addToHashtable(&ht, &p);
                        iPacks++;
                    }else{
                        struct Package *p1 = get(&ht, name);
                        if(strcmp(action, "installed") == 0){
                            //Checks to see if it is removed, to change the status to installed.
                            //If it isnt removed, then it will just say it is installed.
                            if (p1->status == REMOVED){
                                p1->status = INSTALLED;
                                strcpy(p1->rdate, "-");
                                rPacks--;
                            }
                        }else if(strcmp(action, "removed") == 0){
                            if (p1->status == INSTALLED || p1->status == UPDATED){
                                p1->status = REMOVED;
                                strcpy(p1->rdate, date);
                                strcpy(p1->lupdate, date);
                                p1->updates++;
                                rPacks++;
                            }
                        }else{
                            if(p1->status == INSTALLED){
                                p1->status == UPDATED;
                                strcpy(p1->lupdate, date);
                                p1->updates++;
                                uPacks++;
                            }else if(p1->status == UPDATED){
                                strcpy(p1->lupdate, date);
                                p1->updates++;
                            }
                        }
                    }
                }
                validLine = false;
                state = GET_DATE;
                //If i is at the end of the array, add 1 to exit while for sure.
                if(i>=size-1)
                    i++;
                break;
        }
    }
    cInstalled = iPacks - rPacks;
    char rBuf[100000];
    // cat the report to rBuf
    makeReport(rBuf, iPacks, rPacks, uPacks, cInstalled, &ht);
    // 600 allows the owner to read and write, chmod permissions.
    fd = open(report, O_CREAT | O_WRONLY, 0644);
    if (fd == -1){
        printf("Failed to create the report file, or a file with that name already exists..\n");
        return;
    }
    // Write rBuf into the file.
    write(fd, rBuf, strlen(rBuf));
    close(fd);
    printf("Report is generated at: [%s]\n", report);
}

//function to see if the first 3 letters are ins for installed, re for removed and up for upgraded.
bool isAction(char c1, char c2, char c3){
    if(c1 == 'i' && c2 == 'n' && c3 == 's')
        return true;
    if(c1 == 'r' && c2 == 'e')
        return true;
    if(c1 == 'u' && c2 == 'p')
        return true;
    return false;
}

//I know it is not a hashtable, but making the whole thing is kinda tedious, I know
//it is more efficient that way, but I'm trying to complete this with the least help possible.
bool inHashTable(struct Hashtable *ht, char name[]){
    int i;
    for(i = 0; i < ht->nelements; i++){
        if(strcmp(name, ht->array[i].name) == 0){
            return true;
        }
    }
    return false;
}

//Since the index nelements is the next one to be used, we can use it to add the next one.
void addToHashtable(struct Hashtable *ht,struct Package *p){
    ht->array[ht->nelements] = *p;
    ht->nelements++;
}

//Goes through the whole array of packages until it finds one with the same name.
struct Package *get(struct Hashtable *ht, char name[]){
    int i;
    for(i = 0; i < ht->nelements; i++){
        if(strcmp(name, ht->array[i].name)==0){
            return &ht->array[i];
        }
    }
    return NULL;
}

//Creates report, adding the total and then calling htToString
void makeReport(char *report, int iPacks, int rPacks, int uPacks, int cInstalled, struct Hashtable *ht)
{
    strcat(report, "Pacman Packages Report\n");
    strcat(report, "----------------------\n");
    char numBuf[120];
    sprintf(numBuf, "- Installed packages : %i\n- Removed packages   : %i\n- Upgraded packages  : %i\n- Current installed  : %i\n\n", iPacks, rPacks, uPacks, cInstalled);
    strcat(report, numBuf);
    strcat(report, "List of packages\n");
    strcat(report, "----------------------\n");
    htToString(report, ht);
}

//Goes through every element in the table and adds it to report with the specified format.
void htToString(char report[], struct Hashtable *ht){
    int i;
    for (i = 0; i < ht->nelements; i++){
        pToString(report, &ht->array[i]);
        strcat(report, "\n\n");
    }
}


//Adds te package p to the report
void pToString(char report[], struct Package *p){
    strcat(report, "- Package Name        : ");
    strcat(report, p->name);
    strcat(report, "\n");
    strcat(report, "  - Install date      : ");
    strcat(report, p->idate);
    strcat(report, "\n");
    strcat(report, "  - Last update date  : ");
    strcat(report, p->lupdate);
    strcat(report, "\n");
    strcat(report, "  - How many updates  : ");
    // updates is an int, so we need to get it to char to add it to report.
    char numBuf[20];
    sprintf(numBuf, "%d\n", p->updates);
    strcat(report, numBuf);
    strcat(report, "  - Removal date      : ");
    strcat(report, p->rdate);
}
