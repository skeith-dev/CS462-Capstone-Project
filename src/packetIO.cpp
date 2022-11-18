//
// Created by Spencer Keith on 11/17/22.
//

#include "packetIO.h"


void printPacket(char packet[], int packetSize) {

    int length = (int) (sizeof(int) + sizeof(bool) + packetSize);
    std::cout << "[ ";
    for(int i = 0; i < length; i++) {
        std::cout << packet[i];
    }
    std::cout << " ]" << std::endl;

}

void sendPacket(int clientSocket, char packet[], int seqNum, int packetSize) {

    int length = (int) (sizeof(int) + sizeof(bool) + packetSize);
    ssize_t result = send(clientSocket, packet, length, 0);

    if(result != -1) {
        std::cout << "Sent Packet #" << seqNum << ": ";
        printPacket(packet, packetSize);
    } else {
        std::cout << "Failed to send Packet #" << seqNum << "!" << std::endl;
    }

}