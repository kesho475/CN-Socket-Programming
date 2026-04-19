#ifndef COMMON_H
#define COMMON_H
#define PORT 9000
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define MAX_PACKETS 100
#define WINDOW_SIZE 4

// Packet Types
#define DATA 0
#define ACK  1
#define JOIN 2

// Standard Packet Structure used across the whole system
typedef struct {
    int seq;       // Sequence Number
    int type;      // DATA, ACK, or JOIN
    int cid;       // Client ID for multi-client tracking
    char msg[BUFFER_SIZE];
} Packet;

#endif