//
// Created by Spencer Keith on 11/18/22.
//

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include "prompts.h"
#include "fileIO.h"
#include "packetIO.h"
#include "sitErrors.h"

#define FINAL_SEQUENCE_NUMBER -1


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function declarations                          //*****//*****//*****//

void stopAndWaitProtocol(int clientSocket, const std::string& filePath, int fileSize, int numOfPackets, int packetSize, int timeoutInterval, const std::vector<int>& sitErrorsIterations);

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function implementations (including main)      //*****//*****//*****//

int main() {

    //set up TCP socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0); //SOCK_STREAM for TCP
    struct sockaddr_in serverAddress = {0};
    if(clientSocket == -1) {
        perror("Failed to create client socket!");
        exit(EXIT_FAILURE);
    }
    serverAddress.sin_family = AF_INET;

    bool quit; //true for yes, false for no
    do {

        //prompt user for each of the following fields
        //port number of the target server
        int portNum = userIntPrompt("What is the port number of the target server:", 0, 9999);
        serverAddress.sin_port = htons(portNum);
        std::cout << std::endl << "Connecting to server..." << std::endl << std::endl;
        if (connect(clientSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
            std::cout << "Connection to server failed!" << std::endl;
            exit(0);
        }
        //0 for S&W, 1 for SR
        int protocolType = userIntPrompt("Type of protocol, S&W (0) or SR (1):", 0, 1);
        //specified size of packets to be sent
        int packetSize = userIntPrompt("Size of packets:", 1, INT_MAX);
        //user-specified (0+) or default (-1)
        int timeoutInterval = userIntPrompt("Timeout interval, user-specified or ping calculated (-1):", -1, INT_MAX);
        //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
        int slidingWindowSize;
        //ex. (sliding window size = 3) [1, 2, 3] -> [2, 3, 4] -> [3, 4, 5], range = 5
        int rangeOfSeqNums;
        if(protocolType > 0) {
            slidingWindowSize = userIntPrompt("Size of sliding window:", 1, MAX_INPUT);
            rangeOfSeqNums = userIntPrompt("Range of sequence numbers: ", 1, slidingWindowSize / 2);
        }
        //none (0), randomly generated (1), or user-specified (2)
        std::vector<int> sitErrorsIterations;
        int sitErrors = userIntPrompt("Situational errors; none (0), randomly generated (1), or user-specified (2):", 0, 2);
        switch(sitErrors) {
            case 1:
                generateRandomSitErrors(sitErrorsIterations);
                break;
            case 2:
                generateUserSitErrors(sitErrorsIterations);
                break;
            default:
                break;
        }
        //path of file to be sent
        std::string filePath = userStringPrompt("What is the filepath of the file you wish to write FROM:");

        //size of file in bytes
        int fileSize = openFile(filePath);
        //the number of packets necessary to send the whole file
        int numOfPackets = fileSize / packetSize + fileSize % packetSize;

        switch (protocolType) {
            case 0:
                std::cout << std::endl << "Executing Stop & Wait protocol..." << std::endl << std::endl;
                stopAndWaitProtocol(clientSocket, filePath, fileSize, numOfPackets, packetSize, timeoutInterval, sitErrorsIterations);
                break;
            case 1:
                std::cout << std::endl << "Executing Selective Repeat protocol..." << std::endl << std::endl;
                break;
            default:
                break;
        }

        quit = userBoolPrompt("\nWould you like to exit (1), or perform another file transfer (0):");
    } while (!quit);

    return 0;

}

//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Network protocols (algorithms)

void stopAndWaitProtocol(int clientSocket, const std::string& filePath, int fileSize, int numOfPackets, int packetSize, int timeoutInterval, const std::vector<int>& sitErrorsIterations) {

    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;

    std::chrono::system_clock::time_point timerStart;

    char packet[ sizeof(int) + packetSize ];
    char ack[ sizeof(int) ];
    bool outstanding = false;
    int iterator = 0;
    int sitErrorsIterator = 0;

    startTime = std::chrono::system_clock::now();

    while(true) {

        if(iterator > numOfPackets - 1) {
            writeFinalPacket(packet, packetSize);
            sendPacket(clientSocket, packet, FINAL_SEQUENCE_NUMBER, packetSize);
            break;
        }

        if(!outstanding) {
            writeFileToPacket(packet, filePath, fileSize, iterator, packetSize, numOfPackets);
            if(!checkIfDropPacket(sitErrorsIterator, sitErrorsIterations)) {
                sendPacket(clientSocket, packet, iterator, packetSize);
            } else {
                std::cout << "Dropping Packet #" << iterator << std::endl;
            }
            sitErrorsIterator++;
            timerStart = std::chrono::system_clock::now();
            outstanding = true;
        }

        if(recv(clientSocket, ack, (sizeof(int)), MSG_DONTWAIT) != -1) {
            int ackSeqNum;
            std::memcpy(&ackSeqNum, &ack, sizeof(int));
            std::cout << "Received ack #" << ackSeqNum << std::endl;
            iterator = ackSeqNum + 1;
            timerStart = std::chrono::system_clock::now();
            outstanding = false;
        }

        std::chrono::duration<double> timer = std::chrono::system_clock::now() - timerStart;
        if(timer.count() >= timeoutInterval) {
            std::cout << "Timed out! " << timer.count() << " > " << timeoutInterval << " (timeout interval)" << std::endl;
            timerStart = std::chrono::system_clock::now();
            writeFileToPacket(packet, filePath, fileSize, iterator, packetSize, numOfPackets);
            if(!checkIfDropPacket(sitErrorsIterator, sitErrorsIterations)) {
                sendPacket(clientSocket, packet, iterator, packetSize);
            } else {
                std::cout << "Dropping Packet #" << iterator << std::endl;
            }
            sitErrorsIterator++;
        }

    }

    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << std::endl << "Total execution time = " << elapsedSeconds.count() << std::endl;

}