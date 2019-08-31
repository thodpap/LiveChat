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

// Global variables
volatile sig_atomic_t flag = 0;
int clientSocket = 0;
char username[NAME] = {};

void cntrl_c_exit(int sig) {
    flag = 1;
}

void recieveHandler() {
    char rbuffer[SEND] = {};
    while (1) {
        int receive = recv(clientSocket, rbuffer, SEND, 0);
        if (receive > 0) {
            printf("\r%s\n", rbuffer);
            clearOutput();
        } else if (receive == 0) {
            break;
        } 
    }
}

void sendHandler() {
    char msg[MSG] = {};
    while (1) {
        clearOutput();
        while (fgets(msg, MSG, stdin) != NULL) {
            clearArray(msg, MSG);
            if (strlen(msg) == 0) {
                clearOutput();
            } else {
                break;
            }
        }
        send(clientSocket, msg, MSG, 0);
        if (strcmp(msg, "exit") == 0) {
            break;
        }
    }
    cntrl_c_exit(2);
}

int main(){
    signal(SIGINT, cntrl_c_exit);

    // Naming
    printf("Please enter your name: ");
    if (fgets(username, NAME, stdin) != NULL) {
        clearArray(username, NAME);
    }
    if (strlen(username) < 2 || strlen(username) >= NAME-1) {
        printf("\nName must be more than one and less than thirty characters.\n");
        exit(EXIT_FAILURE);
    }

    // Create clientSocket
    clientSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (clientSocket < 0) {
        printf("Fail to create a clientSocket.");
        exit(EXIT_FAILURE);
    }

    // clientSocket information
    struct sockaddr_in serverInfo, clientInfo;
    int sAddrLen = sizeof(serverInfo);
    int cAddrLen = sizeof(clientInfo);
    
    memset(&serverInfo, 0, sAddrLen);
    memset(&clientInfo, 0, cAddrLen);
    
    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("192.168.1.13");//INADDR_ANY; // inet_addr("IP_ADDR");
    serverInfo.sin_port = htons(PORT);

    // Connect to Server
    int con = connect(clientSocket, (struct sockaddr *)&serverInfo, sAddrLen);
    if (con < 0) {
        printf("Connection to Server error!\n");
        exit(EXIT_FAILURE);
    }
    
    // Names
    getsockname(clientSocket, (struct sockaddr*) &clientInfo, (socklen_t*) &cAddrLen);
    getpeername(clientSocket, (struct sockaddr*) &serverInfo, (socklen_t*) &sAddrLen);
    
    printf("Connect to Server: %s:%d\n", inet_ntoa(serverInfo.sin_addr), 
    
    ntohs(serverInfo.sin_port));
    
    printf("You are: %s:%d\n", inet_ntoa(clientInfo.sin_addr), 
    ntohs(clientInfo.sin_port));

    send(clientSocket, username, NAME, 0);

    pthread_t sThread;
    if (pthread_create(&sThread, NULL, (void *) sendHandler, NULL) != 0) {
        printf ("Create pthread error!\n");
        exit(EXIT_FAILURE);
    }

    pthread_t rThread;
    if (pthread_create(&rThread, NULL, (void *) recieveHandler, NULL) != 0) {
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
