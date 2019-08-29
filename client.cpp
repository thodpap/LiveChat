#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#inlcude <netinet/in.h>
#include <arpa/intet.h>

#include <pthread.h>

#include "string.h"
#include "proto.h"

volatile sig_atomic_t flag = 0;
char username[LENGTH_NAME] = {};
int clientSocket = 0;

void ctrl_c_exit(int sig){
    flag = 1;
}

void recvHandler(){
    char msg[LENGTH_SEND] = {};
    while(true){
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
    while(true){
        clearStdout();
        while(fgets(msg,LENGTH_MSG,stdin) != NULL){
            clearArray(msg,LENGTH_MSG);
            if(strlen(message) == 0)
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

    memset(&serverInfo, 0 , sizeof(serverInfo));
    memset(&clientInfo, 0 , sizeof(clientInfo));

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverInfo.sin_port = htons(PORT);

    if(connect(clientSocket, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) < 0){
        printf("Error to Connect to Server\n");
        exit(EXIT_FAILURE);
    }
    // Get Names
    getsockname(clientSocket, (struct sockaddr_in)&clientInfo, (socklen_t *)&(sizeof(clientInfo)));
    getpeername(clientSocket, (struct sockaddr_in)&serverInfo, (socklen_t *)&(sizeof(serverInfo)));

    printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), ntohs(server_info.sin_port));
    printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), ntohs(client_info.sin_port));

    send(clientSocket,username,LENGTH_NAME,0);

    // pthread... for sending and receiving messages using pthread_create and pthread_t

    close(clientSocket);
    return 0;
}
