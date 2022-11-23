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

int sendPacket(int clientSocket, char packet[], int seqNum, int packetSize) {

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
	return result;

}

int sendPacketSW(int clientSocket, char packet[], int iterator, int packetSize) {

    int length = (int) (sizeof(int) + packetSize);
    ssize_t result = send(clientSocket, packet, length, 0);

    if(result != -1) {
        std::cout << "Sent Packet #" << iterator << ": ";
        printPacket(packet, packetSize);
    } else {
        std::cout << "Failed to send Packet #" << iterator << "!" << std::endl;
    }
	
	return result;

}

void sendAck(int clientSocket, int seqNum) {//ACK SIZE == 1 for now

	int ack_packet_size = (int) (sizeof(int) + sizeof(bool) + 1);
	char ack[ack_packet_size];
	ack[sizeof(int)+sizeof(bool)+1] = 6;//ACK value (for fun?)

	char packet_sequence_number_bytes[sizeof(int)];					
	int temp_seq_num = seqNum;
	
	for (int i=sizeof(int)-1; i >= 0; i--) {
		if(temp_seq_num - 127 > 0) {
			packet_sequence_number_bytes[i] = 127;
			temp_seq_num = temp_seq_num - 127;
		} else if (temp_seq_num - 127 < 0){
			packet_sequence_number_bytes[i] = temp_seq_num;
			temp_seq_num = 0;
		} else {
			packet_sequence_number_bytes[i] = 0;
		}
	}

	//write seqNum to ack
	for(int i=0; i<sizeof(int); i++) {
		ack[i] = packet_sequence_number_bytes[i];
	}

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

void sendAckSW(int clientSocket, char ack[], int iterator) {

    int length = sizeof(int);
    ssize_t result = send(clientSocket, ack, length, 0);

    if(result != -1) {
        std::cout << "Send Ack #" << iterator << std::endl;//": "
        //printAck(ack);
    } else {
        std::cout << "Failed to send Ack #" << iterator << "!" << std::endl;
    }

}
