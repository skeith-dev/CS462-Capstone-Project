//
// Created by Spencer Keith on 11/18/22.
//

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include "prompts.h"
#include "fileIO.h"
#include "packetIO.h"

#define FINAL_SEQUENCE_NUMBER (-1)


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function declarations                          //*****//*****//*****//

void stopAndWaitProtocol(int clientSocket, int packetSize, const std::string& filePath);

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function implementations (including main)      //*****//*****//*****//

int main() {

    //set up TCP socket
    struct sockaddr_in serverAddress = {0};
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Failed to create server socket!");
        exit(EXIT_FAILURE);
    }

    //bind server socket to port
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Failed to bind socket to port (setsockopt)!");
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bool quit; //true for yes, false for no
    do {

        //prompt user for each of the following fields
        //port number of the target server
        int portNum = userIntPrompt("What is the port number of the target server:", 0, 9999);
        serverAddress.sin_port = htons(portNum);
        if (bind(serverSocket, (sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) {
            perror("Failed to bind socket to port (bind)!");
            exit(EXIT_FAILURE);
        }
        //0 for S&W, 1 for SR
        int protocolType = userIntPrompt("Type of protocol, S&W (0) or SR (1):", 0, 1);
        //specified size of packets to be sent
        int packetSize = userIntPrompt("Size of packets:", 1, INT_MAX);
        //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
        int slidingWindowSize;
        if(protocolType > 0) {
            slidingWindowSize = userIntPrompt("Size of sliding window:", 1, MAX_INPUT);
        }
        //path of file to be sent
        std::string filePath = userStringPrompt("What is the filepath of the file you wish to write TO:");

        //listen for client connection
        std::cout << std::endl << "Listening for client connection..." << std::endl;
        if (listen(serverSocket, SOMAXCONN) < 0) {
            perror("Failed to listen for client connection!");
            exit(EXIT_FAILURE);
        }

        //wait for connection
        sockaddr_in client = {0};
        socklen_t clientSize = sizeof(client);
        int clientSocket = accept(serverSocket, (sockaddr*) &client, &clientSize);
        if(clientSocket == -1){
            perror("Client socket failed to connect!");
            exit(EXIT_FAILURE);
        }

        switch (protocolType) {
            case 0:
                std::cout << std::endl << "Executing Stop & Wait protocol..." << std::endl << std::endl;
                stopAndWaitProtocol(clientSocket, packetSize, filePath);
                break;
            case 1:
                std::cout << std::endl << "Executing Selective Repeat protocol..." << std::endl << std::endl;
                //executeSRProtocol();
                break;
            default:
                break;
        }

        quit = userBoolPrompt("\nWould you like to exit (1), or perform another file transfer (0):");
    } while(!quit);

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Network protocols (algorithms)

void stopAndWaitProtocol(int clientSocket, int packetSize, const std::string& filePath) {

    char packet[ sizeof(int) + packetSize ];
    char ack[ sizeof(int) ];
    int iterator = 0;
    while(true) {

        if(read(clientSocket, packet, sizeof(packet)) != -1) {

            int packetSeqNum;
            std::memcpy(&packetSeqNum, &packet, sizeof(int));
            if(packetSeqNum == FINAL_SEQUENCE_NUMBER) {
                break;
            }

            std::cout << "Received packet #" << packetSeqNum << "! ";
            printPacket(packet, packetSize);

            if(packetSeqNum <= iterator) {
                std::copy(static_cast<const char*>(static_cast<const void*>(&packetSeqNum)),
                          static_cast<const char*>(static_cast<const void*>(&packetSeqNum)) + sizeof(packetSeqNum),
                          ack);
                sendAck(clientSocket, ack, packetSeqNum);
                appendPacketToFile(packet, packetSize, filePath);
                iterator++;
            } else {
                std::cout << "Received packet is corrupted!" << std::endl;
            }

        }

    }

}