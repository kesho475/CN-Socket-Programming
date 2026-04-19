#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "common.h"

/* * Client-Side Entry Point
 * Initial Setup & Network Boilerplate
 */

int main() {
    int sock;
    struct sockaddr_in serv;
    Packet p;

    // Create UDP Socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket error");
        exit(1);
    }

    // Configure Server Address (we are using localhost for the project)
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    serv.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Initialize and send JOIN request to the server
    memset(&p, 0, sizeof(p));
    p.type = JOIN;

    sendto(sock, &p, sizeof(p), 0,
           (struct sockaddr*)&serv, sizeof(serv));

    printf("client started...\n");
    
    return 0;
}