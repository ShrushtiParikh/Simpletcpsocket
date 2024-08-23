#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080 // Replace with your desired port

void handle_client_communication(int sock) {
    char rx_buffer[128];
    char tx_buffer[128];
    int len;

    while(1) {
        // Receive message from client
        memset(rx_buffer, 0, sizeof(rx_buffer)); 
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            perror("Error occurred during receiving");
            break;
        } else if (len == 0) {
            printf("Connection closed\n");
            break;
        } else {
            rx_buffer[len] = '\0'; // Null-terminate the received data
            printf("Received from Client: %s\n", rx_buffer);
        }

        // Read input from terminal to send to client
        fgets(tx_buffer, sizeof(tx_buffer), stdin);
        int to_write = strlen(tx_buffer);
        while (to_write > 0) {
            int written = send(sock, tx_buffer + (strlen(tx_buffer) - to_write), to_write, 0);
            if (written < 0) {
                perror("Error occurred during sending");
                return;
            }
            to_write -= written;
        }
    }
}

void tcp_server_task(void) {
    int listen_sock;
    struct sockaddr_in server_addr;

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        perror("Unable to create socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Socket unable to bind");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }
    printf("Socket bound, port %d\n", PORT);

    if (listen(listen_sock, 1) < 0) {
        perror("Error occurred during listen");
        close(listen_sock);
        exit(EXIT_FAILURE);
    }

    printf("Socket listening\n");

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock < 0) {
            perror("Unable to accept connection");
            break;
        }

        char addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, addr_str, sizeof(addr_str));
        printf("Socket accepted ip address: %s\n", addr_str);

        handle_client_communication(client_sock);

        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
    }

    close(listen_sock);
}

int main(void) {
    tcp_server_task();
    return 0;
}
