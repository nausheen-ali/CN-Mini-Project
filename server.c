#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <ctype.h> 

#pragma comment(lib, "ws2_32.lib")

#define PORT 9999
#define BUFFER_SIZE 4096 
#define MAX_QUESTIONS 6
#define QUIT_CMD "quit"
#define RESTART_CMD "restart"
#define START_CMD "start"


typedef struct {
    const char *question;
    const char *options[3];
    int correct_index; 
} QuizQuestion;

QuizQuestion quiz_questions[MAX_QUESTIONS] = {
    {"What is the capital of France?", {"Paris", "Rome", "Madrid"}, 0},
    {"What is 5 + 7?", {"10", "12", "15"}, 1},
    {"Which planet is known as the Red Planet?", {"Mars", "Jupiter", "Venus"}, 0},
    {"How many colours are there in a rainbow?", {"5", "6", "7"}, 2},
    {"What is the largest ocean on Earth?", {"Atlantic", "Indian", "Pacific"}, 2},
    {"What data structure does the LIFO principle follow?", {"Queue", "Stack", "Array"}, 1}
};

int current_question = 0;
int score = 0;
int server_running = 1;

void get_question_string(int q_index, char *buffer) {
    if (q_index >= MAX_QUESTIONS) {
        snprintf(buffer, BUFFER_SIZE, "Quiz Finished! Final Score: %d/%d.\n\nType '%s' to play again or '%s' to exit.", 
                 score, MAX_QUESTIONS, RESTART_CMD, QUIT_CMD);
        return;
    }
    
    snprintf(buffer, BUFFER_SIZE, 
            "Question %d/%d: %s\nOptions:\n1. %s\n2. %s\n3. %s\nEnter your answer number (1, 2, or 3):",
            q_index + 1, MAX_QUESTIONS, 
            quiz_questions[q_index].question,
            quiz_questions[q_index].options[0],
            quiz_questions[q_index].options[1],
            quiz_questions[q_index].options[2]);
}

void process_message(char *recv_buf, char *send_buf) {

    char client_msg[BUFFER_SIZE];
    strcpy(client_msg, recv_buf);
    
    for (int i = 0; client_msg[i]; i++) {
        client_msg[i] = tolower(client_msg[i]);
    }

    if (strcmp(client_msg, QUIT_CMD) == 0) {
        server_running = 0;
        snprintf(send_buf, BUFFER_SIZE, "Server received quit command. Shutting down.");
        return;
    }

    if (strcmp(client_msg, START_CMD) == 0 || strcmp(client_msg, RESTART_CMD) == 0) {
        current_question = 0;
        score = 0;
        get_question_string(current_question, send_buf);
        return;
    }

    if (current_question >= MAX_QUESTIONS) {
        get_question_string(current_question, send_buf); 
        return;
    }

    if (client_msg[0] >= '1' && client_msg[0] <= '3' && client_msg[1] == '\0') {
        int answer = client_msg[0] - '0';
        
        QuizQuestion current_q = quiz_questions[current_question];
        char next_msg[BUFFER_SIZE];
        
        if (answer == current_q.correct_index + 1) {
            score++;
            snprintf(send_buf, BUFFER_SIZE, "**Correct!** Your current score is %d.", score);
        } else {
            const char *correct_option = current_q.options[current_q.correct_index];
            snprintf(send_buf, BUFFER_SIZE, "**Incorrect.** The correct answer was **%s**. Your current score is %d.", correct_option, score);
        }
        
        current_question++;
        
        get_question_string(current_question, next_msg);
        strncat(send_buf, "\n\n", BUFFER_SIZE - strlen(send_buf) - 1);
        strncat(send_buf, next_msg, BUFFER_SIZE - strlen(send_buf) - 1);

    } else {
        snprintf(send_buf, BUFFER_SIZE, "Invalid input. Please type 'start', 'restart', 'quit', or an answer number (1, 2, or 3).");
    }
}


int main() {
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server, client;
    int client_len = sizeof(client);
    char recv_buf[BUFFER_SIZE];
    char send_buf[BUFFER_SIZE];
    int recv_len;

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
    printf("Socket created.\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; 
    server.sin_port = htons(PORT);

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return 1;
    }
    printf("UDP Quiz Server listening on port %d...\n", PORT);
    printf("Waiting for initial 'start' message or 'quit'...\n");

    while (server_running) {
        
        if ((recv_len = recvfrom(s, recv_buf, BUFFER_SIZE, 0, (struct sockaddr *)&client, &client_len)) == SOCKET_ERROR) {
            continue; 
        }

        recv_buf[recv_len] = '\0'; 
        
        printf("\nReceived message from %s:%d: %s\n", 
               inet_ntoa(client.sin_addr), ntohs(client.sin_port), recv_buf);

        process_message(recv_buf, send_buf);

        if (sendto(s, send_buf, strlen(send_buf), 0, (struct sockaddr *)&client, client_len) == SOCKET_ERROR) {
            printf("sendto() failed with error code : %d", WSAGetLastError());
        }
        
        if (!server_running) {
            break;
        }
    }

    closesocket(s);
    WSACleanup();
    printf("\nServer shutting down gracefully.\n");

    return 0;
}