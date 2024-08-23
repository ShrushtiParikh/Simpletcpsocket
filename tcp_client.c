#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
#define HOST_IP_ADDR "127.0.0.1" // Replace with the actual server IP address
#define PORT 8080               // Replace with the actual server port

int main(void)
{
    char rx_buffer[1024];
    char tx_buffer[1024];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;
    bool message_printed = false; 

    while (1) {
        struct sockaddr_in dest_addr;
        inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);

        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            perror("Unable to create socket");
            break;
        }
        printf("Socket created, connecting to %s:%d\n", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err != 0) {
            perror("Socket unable to connect");
            break;
        }
        printf("Successfully connected\n");

        while (1) {
            // Read input from the terminal to send to the server
            fgets(tx_buffer, sizeof(tx_buffer), stdin);

            // Send message to server
            int to_write = strlen(tx_buffer);
            while (to_write > 0) {
                int written = send(sock, tx_buffer + (strlen(tx_buffer) - to_write), to_write, 0);
                if (written < 0) {
                    perror("Error occurred during sending");
                    return 1;
                }
                to_write -= written;
            }

            // Receive message from server
            memset(rx_buffer, 0, sizeof(rx_buffer));
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len < 0) {
                perror("recv failed");
                break;
            } else if (len == 0) {
                printf("Connection closed\n");
                break;
            } else {
                rx_buffer[len] = '\0'; // Null-terminate the received data
                printf("Received from server: %s\n", rx_buffer);
            }
        }

        if (sock != -1) {
            printf("Shutting down socket and restarting...\n");
            shutdown(sock, SHUT_RDWR);
            close(sock);
        }
    }

    return 0;
}
