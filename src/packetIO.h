//
// Created by Spencer Keith on 11/17/22.
//

#ifndef UWEC_CS462_CAPSTONE_PROJECT_PACKETIO_H
#define UWEC_CS462_CAPSTONE_PROJECT_PACKETIO_H

#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


/**
 * Prints the contents of a char array packet
 * @param packet The packets whose contents are being printed
 * @param packetSize The size of the packets contents (not including seqNum and valid bytes)
 */
void printPacket(char packet[], int packetSize);

/**
 * Prints the contents of a char array ack
 * @param ack The ack whose contents are being printed
 */
void printAck(char ack[]);

/**
 * Sends char array packet to destination server
 * @param clientSocket The client socket, which sends the packet to the server
 * @param serverAddress The address of the server, which receives the packet from the client socket
 * @param packet The char array being sent from client to server; contains seqNum, valid, and contents
 * @param seqNum The sequence number of the packet being sent to the server
 * @param packetSize The size of the packet CONTENTS; does NOT include seqNum and valid bytes
 */
void sendPacket(int clientSocket, char packet[], int seqNum, int packetSize);

/**
 * Sends char array ack to client
 * @param clientSocket The client socket, which the ack is being sent to
 * @param ack The char array being sent from server to client
 * @param seqNum The sequence number of the packet received from the client
 */
void sendAck(int clientSocket, char ack[], int seqNum);

/**
 * Prints the sliding window
 * @param slidingWindow The sliding window which is being printed
 * @param slidingWindowSize The size of the sliding window
 */
void printWindow(int slidingWindow[], int slidingWindowSize);

#endif //UWEC_CS462_CAPSTONE_PROJECT_PACKETIO_H
