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
        } else { 
            // -1 
        }
    }
}

void sendHandler() {
    char message[MSG] = {};
    while (1) {
        clearOutput();
        while (fgets(message, MSG, stdin) != NULL) {
            clearArray(message, MSG);
            if (strlen(message) == 0) {
                clearOutput();
            } else {
                break;
            }
        }
        send(clientSocket, message, MSG, 0);
        if (strcmp(message, "exit") == 0) {
            break;
        }
    }
    cntrl_c_exit(2);
}

int main()
{
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
    if (clientSocket == -1) {
        printf("Fail to create a clientSocket.");
        exit(EXIT_FAILURE);
    }

    // clientSocket information
    struct sockaddr_in server_info, client_info;
    int s_addrlen = sizeof(server_info);
    int c_addrlen = sizeof(client_info);
    memset(&server_info, 0, s_addrlen);
    memset(&client_info, 0, c_addrlen);
    server_info.sin_family = PF_INET;
    server_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_info.sin_port = htons(8888);

    // Connect to Server
    int err = connect(clientSocket, (struct sockaddr *)&server_info, s_addrlen);
    if (err == -1) {
        printf("Connection to Server error!\n");
        exit(EXIT_FAILURE);
    }
    
    // Names
    getsockname(clientSocket, (struct sockaddr*) &client_info, (socklen_t*) &c_addrlen);
    getpeername(clientSocket, (struct sockaddr*) &server_info, (socklen_t*) &s_addrlen);
    
    printf("Connect to Server: %s:%d\n", inet_ntoa(server_info.sin_addr), 
    
    ntohs(server_info.sin_port));
    
    printf("You are: %s:%d\n", inet_ntoa(client_info.sin_addr), 
    ntohs(client_info.sin_port));

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
