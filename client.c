#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SERVER_PORT 8080
#define SERVER_HOST "127.0.0.1"
#define BUFFER_SIZE 1024

typedef struct packet {
    uint16_t count;
    uint16_t seq_number;
    char data[80];
} packet;

int main(int argc, char *argv[]) {
    int client_socket;
    struct sockaddr_in server_addr;
    packet pkt;
    double ack_loss_ratio;
    char file_name[80];

    // Get the filename and ACK Loss Ratio from the user
    printf("Enter the file name: ");
    fgets(file_name, sizeof(file_name), stdin);
    file_name[strlen(file_name) - 1] = '\0';
    printf("Enter the ACK Loss Ratio (between 0 and 1): ");
    scanf("%lf", &ack_loss_ratio);

    // creating the socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_HOST);

    // Send file
    pkt.count = htons(strlen(file_name));
    pkt.seq_number = htons(0);
    strncpy(pkt.data, file_name, strlen(file_name));

    sendto(client_socket, &pkt, sizeof(packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // recieve file
    FILE *output_file = fopen("out.txt", "w");
    if (!output_file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    int expected_seq_number = 0;

    while (1) {
        socklen_t server_addr_len = sizeof(server_addr);
        int received_bytes = recvfrom(client_socket, &pkt, sizeof(packet), 0, (struct sockaddr *)&server_addr, &server_addr_len);

        // Error checking
        if (received_bytes < 0) {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }

        int count = ntohs(pkt.count);
        int seq_number = ntohs(pkt.seq_number);

        // Process packets
        if (seq_number == expected_seq_number) {
            if (count == 0) {
                printf("End of Transmission Packet with sequence number %d received\n", seq_number);
                break;
            } else {
                fwrite(pkt.data, 1, count, output_file);
                printf("Packet %d received with %d data bytes\n", seq_number, count);
                printf("Packet %d delivered to user\n", seq_number);
                expected_seq_number = 1 - expected_seq_number;
            }
        } else {
            printf("Duplicate packet %d received with %d data bytes\n", seq_number, count);
        }

        // ACK
        if ((double)rand() / (double)RAND_MAX >= ack_loss_ratio) {
            uint16_t ack = htons(seq_number);
            sendto(client_socket, &ack, sizeof(ack), 0, (struct sockaddr *)&server_addr, server_addr_len);
                        printf("ACK %d successfully transmitted\n", seq_number);
        } else {
            printf("ACK %d lost\n", seq_number);
        }
    }

    // Closes file
    fclose(output_file);

    // Close socket
    close(client_socket);

    return 0;
}
