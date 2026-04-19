#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include "common.h"

// Adding a quick XOR encryption layer so wireshark sniffers can't easily read the plaintext payloads
void enc(char *msg) {
    for (int i = 0; i < strlen(msg); i++) {
        msg[i] ^= 0xAA;
    }
}

// Upgrading to support multiple clients. We need to track their addresses and ports.
typedef struct {
    struct sockaddr_in addr;
} Client;

Client list[MAX_CLIENTS];
int total = 0;

// The big upgrade: converting the 1D ack array into a 2D array so we can track
// the ACK state of every individual packet for every specific client simultaneously.
int ack[MAX_CLIENTS][MAX_PACKETS] = {0};

int find(struct sockaddr_in *a) {
    for (int i = 0; i < total; i++) {
        if (list[i].addr.sin_addr.s_addr == a->sin_addr.s_addr &&
            list[i].addr.sin_port == a->sin_port) {
            return i;
        }
    }
    return -1;
}

int add(struct sockaddr_in *a) {
    if (total >= MAX_CLIENTS) return -1;
    list[total].addr = *a;
    printf("new client %d\n", total);
    return total++;
}

/* * Core Go-Back-N Protocol Implementation 
 * Developed by: Shibi Shetty
 * Focus: Sliding window logic, sequence tracking, and timeout-based retransmission
 */

/* * Multi-Client & Security Upgrades
 * Developed by: Samyak Sanklecha
 * Notes: Upgraded Shibi's core engine to handle multiple clients concurrently
 * using a 2D state array, plus added XOR encryption for basic security.
 */
int main() {
    int sock;
    struct sockaddr_in serv, cli;
    socklen_t len = sizeof(cli);
    Packet p;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    serv.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr*)&serv, sizeof(serv));

    printf("server started...\n");
    printf("waiting clients...\n");

    // Wait until both clients are connected before starting the GBN protocol so nobody misses the start
    while (total < 2) {
        recvfrom(sock, &p, sizeof(p), 0,
                 (struct sockaddr*)&cli, &len);

        int id = find(&cli);
        if (id == -1) add(&cli);
    }

    printf("start sending...\n");

    Packet arr[MAX_PACKETS];
    int n = 10;

    for (int i = 0; i < n; i++) {
        arr[i].seq = i;
        arr[i].type = DATA;

        sprintf(arr[i].msg, "msg %d", i);

        enc(arr[i].msg); // 🔐 encrypt before sending

        printf("encrypted %d: %s\n", i, arr[i].msg); 
    }

    struct timeval t;
    t.tv_sec = 2;
    t.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));

    // GBN State Variables: base tracks the oldest un-ACKed packet, 
    // next is the sequence number for the next packet to send.
    int base = 0, next = 0;

    while (base < n) {

        // Sliding Window Mechanism: Only send packets that fall 
        // within the defined WINDOW_SIZE starting from the base.
        while (next < base + WINDOW_SIZE && next < n) {

            for (int c = 0; c < total; c++) {
                
                // Hardcoded packet drop for the presentation to prove our timeout recovery actually works
                if (c == 1 && next == 2) {
                    printf("drop %d for %d\n", next, c);
                    continue;
                }

                sendto(sock, &arr[next], sizeof(Packet), 0,
                       (struct sockaddr*)&list[c].addr,
                       sizeof(list[c].addr));
            }

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

            // Samyak's Update: Instead of blindly resending everything to everyone, we iterate 
            // through the 2D array and ONLY resend to the specific clients that missed the packet (Selective).
            for (int c = 0; c < total; c++) {
                for (int i = base; i < next; i++) {
                    if (ack[c][i] == 0) {

                        sendto(sock, &arr[i], sizeof(Packet), 0,
                               (struct sockaddr*)&list[c].addr,
                               sizeof(list[c].addr));

                        printf("resend %d to %d\n", i, c);
                    }
                }
            }
        } else {
            if (ackp.type == ACK) {
                int cid = find(&cli);
                int s = ackp.seq;

                if (cid != -1) {
                    ack[cid][s] = 1;
                    printf("ack %d from %d\n", s, cid);
                }
            }
        }

        int ok = 1;

        // Check the 2D array to ensure EVERY client got the base packet before sliding the window
        for (int c = 0; c < total; c++) {
            if (ack[c][base] == 0) {
                ok = 0;
                break;
            }
        }

        if (ok) base++;
    }

    printf("done\n");

    close(sock);
    return 0;
}