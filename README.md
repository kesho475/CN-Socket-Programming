# Reliable Group Notification System via UDP

## 📌 Project Overview
This project is custom Reliable UDP implementation built in C. Since standard UDP does not guarantee packet delivery, order, or state tracking, we engineered a hybrid reliable data transfer (RDT) protocol. 

The system combines the sliding-window mechanics of **Go-Back-N (GBN)** with the efficiency of **Selective Retransmission** to reliably broadcast encrypted payloads to multiple concurrent clients, gracefully handling simulated packet loss.

## ✨ Key Features
* **Reliable UDP Transmission:** Ensures sequence integrity and delivery guarantees over a connectionless protocol.
* **Concurrent Multi-Client Support:** The server blocks until the required number of clients join, then tracks individual acknowledgment states for each client using a 2D matrix.
* **Hybrid Error Recovery:** Uses a GBN sliding window to manage network flow, but optimizes timeouts by *selectively* resending only the specific packets dropped by specific clients.
* **Payload Security:** Implements an XOR cipher to encrypt packet payloads, preventing basic Wireshark sniffing.
* **Loss Simulation:** Built-in packet drop logic to demonstrate the timeout and recovery mechanisms dynamically.

## 🛠️ Architecture & Files
* **`common.h`**: Defines the shared network configurations, packet structures, and port definitions.
* **`server.c`**: The sender. Manages the sliding window, tracks multi-client ACKs, handles encryption, and processes selective retransmissions.
* **`client.c`**: The receiver. Connects to the server, decrypts incoming payloads, enforces in-order delivery, and generates corresponding ACKs.

## 🚀 How to Run the Project
**Prerequisites:** A standard C compiler (`gcc`) and a Linux/Unix environment.

1. **Compile the files:**
   ```bash
   gcc server.c -o server
   gcc client.c -o client


1.1. **Start the server:**
     ```bash
      
    ./server
Server will start and wait for the clients to connect.

1.2. **Start the Clients:**
     ```bash
     
     ./client
Open two new terminal windows and run the client executable in both.
(Once both clients join, the server will begin the reliable, encrypted transmission).


## 👥 Team & Contributions
This project was developed collaboratively, with the workload split across network architecture, protocol logic, and scalability.

1. **Shobhit Keshri (SRN: PES2UG24CS475)**
   
`role:`: Designed the foundational network boilerplate and managed the client-side state machine.

`Contributions:`Defined the shared data structures (common.h), established the UDP socket architecture, and implemented the receiver logic (client.c) including sequence tracking and ACK generation. Handled final integration and repository architecture.

2. **Shibi Shetty (SRN: PES2UG24CS467)**
   
`Role:` Engineered the underlying sender logic and mathematical state tracking.

`Contributions:` Developed the core Go-Back-N sender loop in server.c. Handled the implementation of the sliding window, bounded sequence variables (base and next), and the baseline timeout-recovery trigger.

4. **Samyak Sanklecha (SRN: PES2UG24CS436)**

`Role:` Upgraded the base protocol to support multiple concurrent clients and added network security.

`Contributions:` Re-engineered the state tracking to use a 2D matrix (ack[MAX_CLIENTS][MAX_PACKETS]), allowing the server to handle selective retransmission rather than blindly dumping the entire window. Added the XOR payload encryption layer across the system and implemented the demo packet-drop logic.
