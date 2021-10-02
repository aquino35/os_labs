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

// Prototypes for internal functions and utilities
void error(const char *fmt, ...);
int runClient(char *clientName, int numMessages, char **messages);
int runServer();
void serverReady(int signal);

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
    if (forkPID < 0)

        error("ERROR forking client %s", clientName);
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
//#you can create the global variables and functions that you consider necessary***
//*********************************************************************************

#define PORT_NUMBER 44142


bool serverShutdown = false;

void shutdownServer(int signal)
 {

 //Indicate that the server has to shutdown
     serverShutdown = true;
 //Wait for the children to finish
     while (wait(NULL) > 0) {} 
 //Exit
     exit(0);
}

void client(char *clientName, int numMessages, char *messages[])
{
    int sockfd, n;
    char buffer[256];
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    for(int i = 0; i < numMessages; i++) 
    { // This loop ensures there is a socket for every message to be sent
        
        //Open the socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd < 0) error("ERROR opening socket");

        server = gethostbyname("127.0.0.1");
        if(server == NULL) error("ERROR, no such host");
    
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(PORT_NUMBER);
    
        if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR connecting"); //Connect to the server
    
        n = write(sockfd, messages[i], strlen(messages[i])); //For each message, write to the server and read the response
        if(n < 0) error("ERROR writing to socket");
    
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if(n < 0) error("ERROR reading from socket");
    
        //Accept connection and create a child proccess to read and write 
        printf("Client %s(%d): Return message: %s\n", clientName, getpid(), buffer);
        fflush(stdout);
        sleep(1);
    }
    
    close(sockfd); //Close socket
}

void server()
{
    int sockfd, newSock, n;
    socklen_t clientLength;
    char buffer[256];
    struct sockaddr_in serv_addr, addressC;
    
    signal(SIGINT, shutdownServer); //Handle SIGINT so the server stops when the main process kills it
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //Open the socket
    if(sockfd < 0) error("ERROR opening socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT_NUMBER);
    
    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) error("ERROR on binding"); //Bind the socket
    
    listen(sockfd, MAX_MESSAGES);
    clientLength = sizeof(addressC);
    kill(getppid(), SIGUSR1); //Signal server is ready

    //Accept connection and create a child proccess to read and write
    while(!serverShutdown) 
    { // This loop ensures the servers are always waiting to read and send messages

        newSock = accept(sockfd, (struct sockaddr*) &addressC, &addressC);
        if(newSock < 0) error("ERROR on accept");
        int pid = fork();
        if (pid < 0){
            error("ERROR forking server");
        }
        else if (pid == 0) {
            n = read(newSock, buffer, 255);
            if (n < 0) error("ERROR reading from socket");

            printf("Server child(%d): got message: %s\n", getpid(), buffer); //expected output
            fflush(stdout);

            n = write(newSock, buffer, strlen(buffer));
            if (n < 0) error("ERROR writing to socket");
                
            exit(0);
        }
        else {
            close(newSock);
        }

    }
    close(sockfd); //Close socket
}