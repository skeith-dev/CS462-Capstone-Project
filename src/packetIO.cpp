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

	//TODO - this will have to change when checksum is implemented (to include it in the header)
    int length = (int) (sizeof(int) + sizeof(bool) + packetSize);
    ssize_t result = send(clientSocket, packet, length, 0);
	
    if(result != -1) {
		if(seqNum != -1) {
			std::cout << "Sent Packet #" << seqNum << std::endl;  //<< ": "
			//printPacket(packet, packetSize);
		}
    } else {
        std::cout << "Failed to send Packet #" << seqNum << "!" << std::endl;
    }

}

void sendAck(int clientSocket, char ack[], int seqNum) {//ACK SIZE == 1 for now

	//TODO - this will have to change when checksum is implemented (to include it in the header)
    int length = (int) (sizeof(int) + sizeof(bool) + 1);
    ssize_t result = send(clientSocket, ack, length, 0);

    if(result != -1) {
        std::cout << "Sent Ack #" << seqNum << std::endl;
        //printPacket(ack, 1);
    } else {
        std::cout << "Failed to send Ack #" << seqNum << "!" << std::endl;
    }

}