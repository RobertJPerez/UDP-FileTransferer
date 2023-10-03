#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define PACKET_HEADER_SIZE 4
#define PACKET_DATA_SIZE 80
#define TIMEOUT_USEC_MULTIPLIER 10000

typedef struct {
    uint16_t count;
    uint16_t seq_number;
    char data[PACKET_DATA_SIZE];
} Packet;

bool simulate_loss(double loss_ratio) {
    double rnd = (double)rand() / RAND_MAX;
    return rnd < loss_ratio;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <timeout_n> <packet_loss_ratio>\n", argv[0]);
        return 1;
    }

    int timeout_n = atoi(argv[1]);
    double packet_loss_ratio = atof(argv[2]);
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_n * TIMEOUT_USEC_MULTIPLIER;

    srand(time(NULL));

    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    Packet received_packet;
    memset(&received_packet, 0, sizeof(received_packet));
    recvfrom(server_socket, &received_packet, sizeof(received_packet), 0, (struct sockaddr *)&client_addr, &client_addr_size);

    received_packet.count = ntohs(received_packet.count);
    received_packet.seq_number = ntohs(received_packet.seq_number);

    char filename[BUFFER_SIZE];
    strncpy(filename, received_packet.data, received_packet.count);
    filename[received_packet.count] = '\0';

    FILE *input_file = fopen(filename, "r");
    if (!input_file) {
        printf("Error: Unable to open input file.\n");
        close(server_socket);
        return 1;
    }

    Packet data_packet, eot_packet;
    memset(&eot_packet, 0, sizeof(eot_packet));

    uint16_t seq_number = 0;
    ssize_t bytes_read;

    while ((bytes_read = fread(data_packet.data, 1, PACKET_DATA_SIZE, input_file)) > 0) {
        data_packet.count = htons((uint16_t)bytes_read);
        data_packet.seq_number = htons(seq_number);

        while (1) {
            // Loss Simulation
            if (!simulate_loss(packet_loss_ratio)) {
                sendto(server_socket, &data_packet, PACKET_HEADER_SIZE + bytes_read, 0, (struct sockaddr *)&client_addr, client_addr_size);
                printf("Packet %d successfully transmitted with %zd data bytes\n", seq_number, bytes_read);
            } else {
                printf("Packet %d lost\n", seq_number);
            }

            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(server_socket, &readfds);
            int max_sd = server_socket;

            int activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

            if (activity > 0 && FD_ISSET(server_socket, &readfds)) {
                Packet ack_packet;
                        recvfrom(server_socket, &ack_packet, sizeof(ack_packet), 0, (struct sockaddr *)&client_addr, &client_addr_size);

            uint16_t received_seq_number = ntohs(ack_packet.seq_number);

            if (received_seq_number == seq_number) {
                printf("ACK %d received\n", received_seq_number);
                seq_number = (seq_number + 1) % 2;
                break;
            } else {
                printf("Invalid ACK %d received, expected ACK %d\n", received_seq_number, seq_number);
            }
        } else {
            printf("Timeout for ACK %d\n", seq_number);
        }
    }
}

// EOT packet
eot_packet.count = 0;
eot_packet.seq_number = htons(seq_number);
sendto(server_socket, &eot_packet, PACKET_HEADER_SIZE, 0, (struct sockaddr *)&client_addr, client_addr_size);
printf("EOT Packet %d transmitted\n", seq_number);

fclose(input_file);
close(server_socket);
return 0;
}
