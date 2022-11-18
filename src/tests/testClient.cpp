//
// Created by Spencer Keith on 11/17/22.
//

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include "../prompts.h"
#include "../packetIO.h"
#include "../fileIO.h"


int main() {

    //std::string ipAddress = ipAddressPrompt();
    std::string ipAddress = std::string("127.0.0.1");
    //int portNum = portNumPrompt();
    int portNum = 5000;

    //create and setup client socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); //SOCK_STREAM for TCP
    struct sockaddr_in serverAddress = {0};
    if(clientSocket == -1) {
        perror("Failed to create client socket!");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNum);

    if (connect(clientSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
        std::cout << "Connection to server failed!" << std::endl;
        exit(0);
    }

    char packet[15];
    writeFileToPacket(packet, std::string("/Users/spencerkeith/Desktop/School/Spring 2022/CS 462/temp/UWEC-CS462-Capstone-Project/Crash Team Racing: Nitro Fueled Dev Time Rankings.txt"), 1511, 0, 10, 152);
    printPacket(packet, 10);
    sendPacket(clientSocket, packet, 0, 10);

}