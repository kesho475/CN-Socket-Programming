#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include "common.h"

/* * Server-Side Entry Point
 * Initial Socket Binding and Listener Setup
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

    if (bind(sock, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("bind failed");
        exit(1);
    }

    printf("server started...\n");
    printf("waiting clients...\n");
    
    // GBN and Multi-client logic to be implemented by Shibi and Samyak
    return 0;
}