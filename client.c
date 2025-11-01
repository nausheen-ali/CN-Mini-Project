#include <stdio.h>
#include <winsock2.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define PORT 9999
#define BUFFER_SIZE 4096 
#define QUIT_CMD "quit"

int main() {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    char message[BUFFER_SIZE];
    char server_reply[BUFFER_SIZE]; 
    int server_len = sizeof(server);
    
    printf("\nInitializing Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    printf("Initialized.\n");

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket : %d", WSAGetLastError());
        return 1;
    }
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(PORT);

    printf("Welcome to the UDP Quiz Client.\n");
    printf("Type 'start' to begin, '%s' to exit.\n", QUIT_CMD);

    strcpy(message, "start");
    
    int client_running = 1;

    while (client_running) {
        if (sendto(s, message, strlen(message), 0, (struct sockaddr *)&server, server_len) == SOCKET_ERROR) {
            printf("sendto() failed with error code : %d\n", WSAGetLastError());
            client_running = 0;
            continue;
        }

        printf("\nWaiting for server response...\n");
        int recv_len = recvfrom(s, server_reply, BUFFER_SIZE, 0, (struct sockaddr *)&server, &server_len);
        
        if (recv_len == SOCKET_ERROR) {
            printf("recvfrom() failed with error code : %d\n", WSAGetLastError());
            client_running = 0;
            continue;
        }
        
        server_reply[recv_len] = '\0'; 
        
        printf("\n----------------------------------------\n");
        printf("%s\n", server_reply);
        printf("----------------------------------------\n");

        if (strstr(server_reply, "Server received quit command. Shutting down.")) {
            printf("Server has shut down. Exiting client.\n");
            break;
        }

        printf("Your input: ");
        if (fgets(message, BUFFER_SIZE, stdin) == NULL) {
            break; 
        }
        
        message[strcspn(message, "\n")] = 0; 

        if (strcmp(message, QUIT_CMD) == 0) {
            printf("Sending quit command to server and exiting client.\n");
            sendto(s, message, strlen(message), 0, (struct sockaddr *)&server, server_len);
            client_running = 0;
        }
    }

    closesocket(s);
    WSACleanup();
    printf("Client exiting.\n");
    return 0;
}