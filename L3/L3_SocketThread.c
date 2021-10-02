#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/un.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
// Prototypes for internal functions and utilities
void error(const char *fmt, ...);
int runClient(char *clientName, int numMessages, char **messages);
int runServer();
void serverReady(int signal);

pthread_t tidServer[2]; 

bool serverIsReady = false;

// Prototypes for functions to be implemented by students
void server();
void client(char *clientName, int numMessages, char *messages[]);

void verror(const char *fmt, va_list argp)
{
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, argp);
    fprintf(stderr, "\n");
}

void error(const char *fmt, ...)
{
    va_list argp;
    va_start(argp, fmt);
    verror(fmt, argp);
    va_end(argp);
    exit(1);
}

int runServer(int port) {
    int forkPID = fork();
    if (forkPID < 0)
    error("ERROR forking server");
    else if (forkPID == 0) {
        server();
        exit(0);
    } else {
        printf("Main: Server(%d) launched...\n", forkPID);
    }
    return forkPID;
}

int runClient(char *clientName, int numMessages, char **messages) {
    fflush(stdout);
    printf("Launching client %s...\n", clientName);
    int forkPID = fork();
    if (forkPID < 0) error("ERROR forking client %s", clientName);
    else if (forkPID == 0) {
        client(clientName, numMessages, messages);
        exit(0);
    }
    return forkPID;
}

void serverReady(int signal) {
    serverIsReady = true;
}

#define NUM_CLIENTS 2
#define MAX_MESSAGES 5
#define MAX_CLIENT_NAME_LENGTH 10

struct client {
    char name[MAX_CLIENT_NAME_LENGTH];
    int num_messages;
    char *messages[MAX_MESSAGES];
    int PID;
};

// Modify these to implement different scenarios
struct client clients[] = {
    {"Uno", 3, {"Mensaje 1-1", "Mensaje 1-2", "Mensaje 1-3"}},
    {"Dos", 3, {"Mensaje 2-1", "Mensaje 2-2", "Mensaje 2-3"}}
};

int main() {
    signal(SIGUSR1, serverReady);
    int serverPID = runServer(getpid());
    while(!serverIsReady) {
        sleep(1);
    }
    printf("Main: Server(%d) signaled ready to receive messages\n", serverPID);
    
    for (int client = 0 ; client < NUM_CLIENTS ; client++) {
        clients[client].PID = runClient(clients[client].name, clients[client].num_messages,
                                        clients[client].messages);
    }
    
    for (int client = 0 ; client < NUM_CLIENTS ; client++) {
        int clientStatus;
        printf("Main: Waiting for client %s(%d) to complete...\n", clients[client].name, clients[client].PID);
        pthread_join(tidServer[client],NULL);
        waitpid(clients[client].PID, &clientStatus, 0);
        printf("Main: Client %s(%d) terminated with status: %d\n",
               clients[client].name, clients[client].PID, clientStatus);
    }
    
    printf("Main: Killing server (%d)\n", serverPID);
    kill(serverPID, SIGINT);
    int statusServer;
    pid_t wait_result = waitpid(serverPID, &statusServer, 0);
    printf("Main: Server(%d) terminated with status: %d\n", serverPID, statusServer);
    return 0;
}


//*********************************************************************************
//**************************** EDIT FROM HERE *************************************
//#you can create global variables and functions that you consider necessary***
//*********************************************************************************


#define PORT_NUMBER 46370

bool serverShutdown = false;

void shutdownServer(int signal) 
{
    //Indicate that the server has to shutdown
    serverShutdown = true;
    //Wait for the children to finish
    while(wait(NULL) > 0);
    //Exit
    exit(0);
}

void client(char *clientName, int numMessages, char *messages[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    server = gethostbyname("127.0.0.1");

    if (server == NULL)
    {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    } 

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(PORT_NUMBER);

    for(int i = 0; i < numMessages; i++)
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) error("ERROR opening socket c");

        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
            error("ERROR connecting c");
            
        strcpy(buffer, messages[i]);
        n = write(sockfd,buffer,strlen(messages[i]));
        if (n < 0) error("ERROR writing to socket c");

        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket c");
        
        printf("Client %s(%d): Return message: %s\n", clientName, getpid(), buffer);   
        fflush(stdout);
        sleep(1);
        close(sockfd);
    }
}

void *pthreadServer(void * arguments)
{
    int n;
    char buffer[256];
    uintptr_t newsockfd = (uintptr_t*) arguments;

    // read socket
    n = read(newsockfd, buffer, 255);
    if (n < 0) error("ERROR reading from socket s");

    //expected output
    printf("Server child(%d): got message: %s\n", getpid(), buffer); //expected output 

    // write socket
    n = write(newsockfd, buffer, strlen(buffer));
    fflush(stdout);
    if (n < 0) error("ERROR writing to socket");

    close(newsockfd);
    pthread_exit(0);
}  

void server()
{
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    pthread_attr_t attrServer; /* set of attributes for the thread */

    //Handle SIGINT so the server stops when the main process kills it
    signal(SIGINT, (void*)shutdownServer);

    // Open the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0 )error("Error opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT_NUMBER);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        error("Error on binding socket");
    }

    listen(sockfd, MAX_MESSAGES);

    // Signal server is ready
    kill(getppid(), SIGUSR1);
    clilen = sizeof(cli_addr);

    // Accept connection and create a PTHREAD_T
    int i;
    while(!serverShutdown) 
    { 
        // Accepting connection
        newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr,&clilen);
        if (newsockfd < 0) error("Error on accept");

        pthread_attr_init(&attrServer);/* Get the default attributes */
        pthread_create(&tidServer[i++], &attrServer, pthreadServer, (void*)newsockfd);
    }
    // Close socket
    sleep(1);
    close(sockfd);
}