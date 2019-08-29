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

typedef struct ClientNode{
    int sock;
    struct ClientNode* next;
    struct ClientNode* prev;
    char ip[16];
    char name[31];
}ClientList;

ClientList *node,*copy;

ClientList *newNode(int socket,char *ip){
    ClientList *np = new ClientList;
    np->sock = socket;
    np->next = NULL;
    np->prev = NULL;
    strncpy(np->ip, ip, 16);
    strncpy(np->name, "NULL", 5);
    return np;
}
// I need a function that can send a msg to all clients
void sendEverywhere(ClientList *l, char *msg){
    ClientList *temp = node;
    while(temp != NULL){
        // Except the same client, then
        if(l->sock != temp->sock){
            printf("Send to socket %d: \"%s\" \n", temp->sock, msg);
            send(temp->sock,msg,LENGTH_SEND,0);
        }
        temp = temp->prev;
    }
}
void clientHandler(void *c){
    int flag2 = 0;
    char username[LENGTH_NAME] = {};
    char rbuffer[LENGTH_MSG] = {}; // receive buffer
    char sbuffer[LENGTH_SEND] = {}; // send buffer
    
    ClientList *client = (ClientList *)c;
    //return NULL;
}
int main(){
//     It might fix some issues;
//     signal(SIGINT, catch_ctrl_c_and_exit);
    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printf("Fail to create a socket.");
        exit(EXIT_FAILURE);
    }
    
    // Socket information
    struct sockaddr_in serverInfo, clientInfo;
    int sLen = sizeof(serverInfo);
    int cLen = sizeof (clientInfo);

    memset(&serverInfo, 0, sLen);
    memset(&clientInfo, 0, cLen);

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = INADDR_ANY;
    serverInfo.sin_port = htons(PORT);
    
    // Bind and Listen
    if(bind(serverSocket, (struct sockaddr *)&serverInfo, sLen) < 0){
        printf("Error in binding\n");
        exit(EXIT_FAILURE);
    }
    if(listen(serverSocket, 5) < 0){
        printf("Error in listening\n");
        exit(EXIT_FAILURE);
    }
    
    // Print Server IP
    getsockname(serverSocket, (struct sockaddr*) &serverInfo, (socklen_t*)&sLen);
    printf("Start Server on: %s:%d\n", inet_ntoa(serverInfo.sin_addr), ntohs(serverInfo.sin_port));
    
    // Initial linked list for clients
    node = newNode(serverSocket, inet_ntoa(serverInfo.sin_addr));
    copy = node;

    while(true){
        // Accept the socket
        int clientSocket = accept(serverSocket,(struct sockaddr*) &clientInfo, (socklen_t*) &cLen);

        // print ip
        getpeername(clientSocket, (struct sockaddr*) &clientInfo, (socklen_t*) &cLen);
        printf("Client %s:%d come in.\n", inet_ntoa(clientInfo.sin_addr), ntohs(clientInfo.sin_port));
        
        // Link the Clients, adding a link to the new client
        ClientList *c = newNode(clientSocket,inet_ntoa(clientInfo.sin_addr));
        c->next = copy;
        copy->prev = c;
        copy = c;
        
        // Create the thread
        pthread_t id;
        if (pthread_create(&id, NULL,(void *) clientHandler, (void* )c) != 0) {
            perror("Create pthread error!\n");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
