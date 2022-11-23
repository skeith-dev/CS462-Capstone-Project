//
// Created by Spencer Keith on 11/17/22.
//

#include "packetIO.h"


void printPacket(char packet[], int packetSize) {

    int length = (int) (sizeof(int) + packetSize);
    std::cout << "[ ";
    for(int i = 0; i < length; i++) {
        std::cout << packet[i];
    }
    std::cout << " ]" << std::endl;

}

void printAck(char ack[]) {

    std::cout << "[ ";
    for(int i = 0; i < sizeof(int); i++) {
        std::cout << ack[i];
    }
    std::cout << " ]" << std::endl;

}

void sendPacket(int clientSocket, char packet[], int seqNum, int packetSize) {

    int length = (int) (sizeof(int) + packetSize);
    ssize_t result = send(clientSocket, packet, length, 0);

    if(result != -1) {
        std::cout << "Sent Packet #" << seqNum << ": ";
        printPacket(packet, packetSize);
    } else {
        std::cout << "Failed to send Packet #" << seqNum << "!" << std::endl;
    }

}

void sendAck(int clientSocket, char ack[], int seqNum) {

    int length = sizeof(int);
    ssize_t result = send(clientSocket, ack, length, 0);

    if(result != -1) {
        std::cout << "Send Ack #" << seqNum << ": ";
        printAck(ack);
    } else {
        std::cout << "Failed to send Ack #" << seqNum << "!" << std::endl;
    }

}

void printWindow(int slidingWindow[], int slidingWindowSize) {

    std::cout << "Sliding Window: [ ";
    for(int i = 0; i < slidingWindowSize; i++) {
        std::cout << slidingWindow[i];
    }
    std::cout << " ]" << std::endl;

}