#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define TOKEN_DELIMETERS " \t\r\n\a"
#define DEFAULT_SIZE 1024

char *history[DEFAULT_SIZE];
char *argv[DEFAULT_SIZE];
int hist_count = 0;
int hist_num = 0;
int position = 0;
short isHistNum = 0;

void read_line(char *line)
{
    if(isHistNum){
        line = memcpy(line,history[hist_num-1],DEFAULT_SIZE);
        printf("\n");
    }
    else{
        gets(line);
    }
    isHistNum = 0;
    memcpy(history[hist_count],line,DEFAULT_SIZE);
    hist_count++;
}

void parse_line(char *line,char *argv[])
{
    char *token;
    position = 0;
    token = strtok(line,TOKEN_DELIMETERS);
    while(token!=NULL){
        argv[position] = token;
        position++;
        token =strtok(NULL,TOKEN_DELIMETERS);
    }
}

void nat_help()
{
    printf("\n Welcome to the OSL CLI!! \n ");
    printf("\n In use the commands <history, cd and exit to navigate.> \n");
}

void nat_cd(char *argv[])
{
    chdir(argv[0]);
}


int nat_history(char *argv[]){
    if(position==2){
        hist_num = atoi(argv[1]);
        for(int k = 1; k<=hist_count;k++){
            if(hist_num == k){
                isHistNum = 1;
            }
        }
    }
    if(isHistNum==0){
        for(int i =0; i <hist_count;i++)
            printf(" %d %s\n",(i),history[i]);
    }
    return 1;
}

void execute(char *argv[]){

    // Check if command is valid as a native command
    char terminate[10];	strcpy(terminate, "exit");
    char hist[10];	strcpy(hist, "history");
    char help[10];	strcpy(help, "help");
    char cd[10];	strcpy(cd, "cd");
    char command[300];

    if(strcmp(argv[0], terminate) == 0){
        exit(0);
    }else if(strcmp(argv[0], hist) == 0){
        nat_history(argv);
    }else if(strcmp(argv[0], cd) == 0){
        //nat_cd(argv);
        if (position==2){
            chdir(argv[1]); //nat_cd
        }
    }else if(strcmp(argv[0], help) == 0){
        nat_help();
    }
        //Native commands
    else{
        //Execute any other command. Uses child process.
        strcpy(command, argv[0]);
        system(command);
    }
}

int main(int argc, char *argv[]){
    int valid = 0;
    char *line = (char*)malloc(DEFAULT_SIZE);
    for(int i = 0;i<DEFAULT_SIZE;i++)
        history[i] = (char*)malloc(DEFAULT_SIZE);
    long size;
    char *buf;
    char *ptr;
    while(1){
        printf("Shell->");
        read_line(line);
        parse_line(line,argv);
        execute(argv);
        for(int j = 0; j< argc;j++)
        {
            argv[j] = NULL;
        }
    }
}