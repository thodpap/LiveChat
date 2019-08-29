#ifndef LIST
#define LIST

typedef struct ClientNode {
    int sock;
    struct ClientNode* prev;
    struct ClientNode* link;
    char ip[16];
    char name[31];
}ClientList;

ClientList *newNode(int socket, char* ip) {
    ClientList *client = (ClientList *)malloc( sizeof(ClientList) );
    client->sock = socket;
    client->prev = NULL;
    client->link = NULL;
    strncpy(client->ip, ip, 16);
    strncpy(client->name, "NULL", 5);
    return client;
}

#endif // LIST
