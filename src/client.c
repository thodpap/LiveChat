#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "proto.h"
#include "string.h"

volatile sig_atomic_t flag = 0;
char username[LENGTH_NAME] = {};
int clientSocket = 0;

void ctrl_c_exit(int sig){
    flag = 1;
}

void recvHandler(){
    char msg[LENGTH_SEND] = {};
    while(1){
        int receive = recv(clientSocket,msg,LENGTH_SEND,0); //length of the message
        if(receive > 0){
            printf("\r%s\n",msg); // \r is to print with the cursor full left
            clearStdout();
        }
        else if(receive == 0) break;
    }
}
void sendHandler(){
    char msg[LENGTH_MSG] = {};
    while(1){
        clearStdout();
        while(fgets(msg,LENGTH_MSG,stdin) != NULL){
            clearArray(msg,LENGTH_MSG);
            if(strlen(msg) == 0)
                clearStdout();
            else
                break;
        }
        // Ready to send
        send(clientSocket,msg,LENGTH_MSG,0);
        if(strcmp(msg, "exit") == 0)
            break;
    }
    ctrl_c_exit(2);
}
int main(int argc,const char *argv[]){
    signal(SIGINT,ctrl_c_exit);
    printf("Enter me you name: ");
    if(fgets(username, LENGTH_NAME, stdin) != NULL)
        clearArray(username,LENGTH_NAME);
    // I suppose that a valid name is provided
    // An invalid name is with less than 2 characters, or more than LENGTH_NAME - 1 characters

    clientSocket = socket(AF_INET,SOCK_STREAM,0);
    if(clientSocket < 0){
        printf("Failed to create the socket\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in serverInfo,clientInfo;
    int sLen = sizeof(serverInfo);
    int cLen = sizeof(clientInfo);

    memset(&serverInfo, 0 , sLen);
    memset(&clientInfo, 0 , cLen);

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverInfo.sin_port = htons(PORT);

    if(connect(clientSocket, (struct sockaddr *)&serverInfo, sLen) < 0){
        printf("Error to Connect to Server\n");
        exit(EXIT_FAILURE);
    }
    // Get Names
    getsockname(clientSocket, (struct sockaddr *)&clientInfo, (socklen_t *)&cLen);
    getpeername(clientSocket, (struct sockaddr *)&serverInfo, (socklen_t *)&sLen);

    printf("Connect to Server: %s:%d\n", inet_ntoa(serverInfo.sin_addr), ntohs(serverInfo.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));

    send(clientSocket,username,LENGTH_NAME,0);

    pthread_t sendThread;
    if (pthread_create(&sendThread, NULL, (void *)sendHandler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    pthread_t recThread;
    if (pthread_create(&recThread, NULL, (void *)recvHandler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        if(flag) {
            printf("\nBye\n");
            break;
        }
    }
    close(clientSocket);
    return 0;
}
