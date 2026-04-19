#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include "common.h"

/* * Core Go-Back-N Protocol Implementation 
 * Developed by: Shibi Shetty
 * Focus: Sliding window logic, sequence tracking, and timeout-based retransmission
 */

int main() {
    int sock;
    struct sockaddr_in serv, cli;
    socklen_t len = sizeof(cli);

    // Initialize UDP Socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Bind to the specified port on all available interfaces
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    serv.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr*)&serv, sizeof(serv));

    printf("server started...\n");
    printf("waiting clients...\n");

    // Phase 1 implementation: Wait for a single client to establish the GBN baseline
    recvfrom(sock, &p, sizeof(p), 0, (struct sockaddr*)&cli, &len);
    printf("client connected.\n");
    printf("start sending...\n");

    Packet arr[MAX_PACKETS];
    int n = 10; // Total packets to send for testing
    int ack_single[MAX_PACKETS] = {0}; // 1D array to track ACKs for this single client

    // Pre-compute and package the data (Encryption will be handled later)
    for (int i = 0; i < n; i++) {
        arr[i].seq = i;
        arr[i].type = DATA;
        sprintf(arr[i].msg, "msg %d", i);
        printf("prepared %d: %s\n", i, arr[i].msg); 
    }

    // Configure socket timeout for the GBN retransmission trigger
    struct timeval t;
    t.tv_sec = 2;
    t.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));

    // GBN State Variables: base tracks the oldest un-ACKed packet, 
    // next is the sequence number for the next packet to send.
    int base = 0, next = 0;

    // Main GBN Sender Loop
    while (base < n) {

        // Sliding Window Mechanism: Only send packets that fall 
        // within the defined WINDOW_SIZE starting from the base.
        while (next < base + WINDOW_SIZE && next < n) {
            sendto(sock, &arr[next], sizeof(Packet), 0,
                   (struct sockaddr*)&cli, sizeof(cli));
            printf("sent %d\n", next);
            next++;
        }

        Packet ackp;
        int r = recvfrom(sock, &ackp, sizeof(ackp), 0,
                         (struct sockaddr*)&cli, &len);

        // Protocol Recovery: If a timeout occurs, we fall back 
        // to the GBN strategy and resend all unacknowledged packets in the window.
        if (r < 0) {
            printf("timeout resend...\n");
            for (int i = base; i < next; i++) {
                if (ack_single[i] == 0) {
                    sendto(sock, &arr[i], sizeof(Packet), 0,
                           (struct sockaddr*)&cli, sizeof(cli));
                    printf("resend %d\n", i);
                }
            }
        } else {
            // Process incoming ACK
            if (ackp.type == ACK) {
                int s = ackp.seq;
                ack_single[s] = 1;
                printf("ack %d\n", s);
            }
        }

        // Slide the window forward if the base packet was acknowledged
        if (ack_single[base] == 1) {
            base++;
        }
    }

    printf("done\n");
    close(sock);
    return 0;
}