#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "common.h"

// Security Layer Added by Samyak Sanklecha: Basic XOR encryption
void enc(char *msg) {
    for (int i = 0; i < strlen(msg); i++) {
        msg[i] ^= 0xAA;
    }
}

/* * Client-Side Receiver Logic
 * Developed by: Shobhit Keshri
 * Focus: State machine for receiving packets and handling ACKs in sequence.
 */

int main() {
    int sock;
    struct sockaddr_in serv;
    Packet p, ack;

    // Create UDP Socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket error");
        exit(1);
    }

    // Configure Server Address (we are using Localhost for this project)
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    serv.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Initialize and send JOIN request to the server
    memset(&p, 0, sizeof(p));
    p.type = JOIN;

    sendto(sock, &p, sizeof(p), 0,
           (struct sockaddr*)&serv, sizeof(serv));

    printf("client started...\n");

    // Receiver state: tracking the exact sequence number we need next
    int expect = 0; 

    // Main event loop for the receiver
    while (1) {
        memset(&p, 0, sizeof(p));

        // Block and wait for a packet from the server
        int n = recvfrom(sock, &p, sizeof(p), 0, NULL, NULL);
        if (n < 0) continue;

        if (p.type == DATA) {

            // Samyak's Decryption Hook
            enc(p.msg); // decrypt
            
            // Check if this is the exact packet we are expecting (in-order delivery)
            if (p.seq == expect) {
                
                // Print received message (Decryption to be added by Samyak)
                printf("received %d: %s\n", p.seq, p.msg); 

                // Construct and send the positive ACK
                ack.seq = p.seq;
                ack.type = ACK;

                sendto(sock, &ack, sizeof(ack), 0,
                       (struct sockaddr*)&serv,
                       sizeof(serv));

                expect++; // Slide our receiver window forward

            } else {
                
                // Out-of-order packet detected! 
                // To keep the core GBN logic intact, re-ACK the last good packet we got.
                printf("skip %d (want %d)\n", p.seq, expect);

                ack.seq = expect - 1;
                ack.type = ACK;

                sendto(sock, &ack, sizeof(ack), 0,
                       (struct sockaddr*)&serv,
                       sizeof(serv));
            }
        }
    }

    close(sock);
    return 0;
}