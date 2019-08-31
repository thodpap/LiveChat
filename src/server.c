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
#include "server.h"

// Global variables 
int serverSocket = 0, clientSocket = 0;
ClientList *root, *recent; // recent is copy

void cntrl_c_exit(int sig) {
    ClientList *temp;
    while (root != NULL) {
        printf("\nClose clientSocketfd: %d\n", root->sock);
        close(root->sock); // close all clientSocket include serverclientSocket
        temp = root;
        root = root->link;
        free(temp);
    }
    printf("Bye\n");
    exit(EXIT_SUCCESS);
}

void sendEveryone(ClientList *client, char *msg) {
    ClientList *temp = root->link;
    while (temp != NULL) {
        if (client->sock != temp->sock) { // all clients except itself.
            printf("Send to sockfd %d: \"%s\" \n", temp->sock, msg);
            send(temp->sock, msg, SEND, 0);
        }
        temp = temp->link;
    }
}

void clientHandler(void *p_client) {
    int leave_flag = 0;
    char username[NAME] = {};
    char recv_buffer[MSG] = {};
    char send_buffer[SEND] = {};
    ClientList *client = (ClientList *)p_client;

    // Naming
    if (recv(client->sock, username, NAME, 0) <= 0 || strlen(username) < 2 || strlen(username) >= NAME-1) {
        printf("%s didn't input name.\n", client->ip);
        leave_flag = 1;
    } else {
        strncpy(client->name, username, NAME);
        printf("%s(%s)(%d) join the chatroom.\n", client->name, client->ip, client->sock);
        sprintf(send_buffer, "%s joined the chatroom.", client->name, client->ip);
        sendEveryone(client, send_buffer);
    }

    // Conversation
    while (1) {
        if (leave_flag) {
            break;
        }
        int receive = recv(client->sock, recv_buffer, MSG, 0);
        if (receive > 0) {
            if (strlen(recv_buffer) == 0) {
                continue;
            }
            sprintf(send_buffer, "%s：%s", client->name, recv_buffer);
        } else if (receive == 0 || strcmp(recv_buffer, "exit") == 0) {
            printf("%s left the chat.\n", client->name);// client->ip, client->sock);
            sprintf(send_buffer, "%s(%s) left the chat.", client->name, client->ip);
            leave_flag = 1;
        } else {
            printf("Fatal Error: -1\n");
            leave_flag = 1;
        }
        sendEveryone(client, send_buffer);
    }

    // Remove Node
    close(client->sock);
    if (client == recent) { // remove an edge node
        recent = client->prev;
        recent->link = NULL;
    } else { // remove a middle node
        client->prev->link = client->link;
        client->link->prev = client->prev;
    }
    free(client);
}

int main(){
    signal(SIGINT, cntrl_c_exit);

    // Create clientSocket
    serverSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (serverSocket == -1) {
        printf("Fail to create a clientSocket.");
        exit(EXIT_FAILURE);
    }

    // clientSocket information
    struct sockaddr_in serverInfo, clientInfo;
    int s_addrlen = sizeof(serverInfo);
    int c_addrlen = sizeof(clientInfo);
    
    memset(&serverInfo, 0, s_addrlen);
    memset(&clientInfo, 0, c_addrlen);

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = inet_addr("192.168.1.13"); //INADDR_ANY; // inet_addr("IP_ADDR");
    serverInfo.sin_port = htons(PORT);
    
    // Bind and Listen
    bind(serverSocket, (struct sockaddr *)&serverInfo, s_addrlen);
    listen(serverSocket, 5);
//     if(b < 0 || l <= 0){
//         printf("Error on bind or listen\n");
//         exit(EXIT_FAILURE);
//     }    

    // Print Server IP
    getsockname(serverSocket, (struct sockaddr*) &serverInfo, (socklen_t*) &s_addrlen);
    printf("Start Server on: %s:%d\n", inet_ntoa(serverInfo.sin_addr), ntohs(serverInfo.sin_port));

    // Initial linked list for clients
    root = newNode(serverSocket, inet_ntoa(serverInfo.sin_addr));
    recent = root;

    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*) &clientInfo, (socklen_t*) &c_addrlen);

        // Print Client IP
        getpeername(clientSocket, (struct sockaddr*) &clientInfo, (socklen_t*) &c_addrlen);
        printf("Client %s:%d come in.\n", inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));

        // Append linked list for clients
        ClientList *c = newNode(clientSocket, inet_ntoa(clientInfo.sin_addr));
        c->prev = recent;
        recent->link = c;
        recent = c;

        pthread_t id;
        if (pthread_create(&id, NULL, (void *)clientHandler, (void *)c) != 0) {
            perror("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
