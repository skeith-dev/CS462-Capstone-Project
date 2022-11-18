//
// Created by Spencer Keith on 11/18/22.
//

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include "prompts.h"
#include "fileIO.h"

#define FINAL_SEQUENCE_NUMBER -1


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function declarations                          //*****//*****//*****//

void stopAndWaitProtocol();

void selectiveRepeatProtocol();

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
        if(protocolType > 0) {
            slidingWindowSize = userIntPrompt("Size of sliding window:", 1, MAX_INPUT);
        }
        //none (0), randomly generated (1), or user-specified (2)
        int situationalErrors = userIntPrompt("Situational errors; none (0), randomly generated (1), or user-specified (2):", 0, 2);
        switch(situationalErrors) {
            case 1:
                //generateRandomSitErrors();
                break;
            case 2:
                //generateUserSituationalErrors();
                break;
            default:
                break;
        }
        //path of file to be sent
        std::string filePath = userStringPrompt("What is the filepath of the file you wish to write FROM:");

        //size of file in bytes
        int fileSize = openFile(filePath);
        //the range of sequence numbers necessary to send the whole file
        int fileSizeRangeOfSeqNums = fileSize / packetSize + fileSize % packetSize;
        //ex. (sliding window size = 3) [1, 2, 3] -> [2, 3, 4] -> [3, 4, 5], range = 5
        std::stringstream ss;
        ss << "Range of sequence numbers (" << fileSizeRangeOfSeqNums << " required to send entire file):";
        int rangeOfSeqNums = userIntPrompt(ss.str(), 1, fileSizeRangeOfSeqNums);

        switch (protocolType) {
            case 0:
                std::cout << std::endl << "Executing Stop & Wait protocol..." << std::endl << std::endl;
                //executeSAWProtocol(clientSocket, serverAddress);
                break;
            case 1:
                std::cout << std::endl << "Executing Selective Repeat protocol..." << std::endl << std::endl;
                //executeSRProtocol();
                break;
            default:
                break;
        }

        quit = userBoolPrompt("Would you like to exit (1), or perform another file transfer (0):");
    } while (!quit);

    return 0;

}