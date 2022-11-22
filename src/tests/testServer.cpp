//
// Created by Spencer Keith on 11/17/22.
//

#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include "../prompts.h"
#include "../packetIO.h"
#include "../fileIO.h"


int main() {

    //std::string ipAddress = ipAddressPrompt();
    std::string ipAddress = std::string("127.0.0.1");
    //int portNum = portNumPrompt();
    int portNum = 5000;

    //create and setup server socket
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
    serverAddress.sin_addr.s_addr = INADDR_ANY; //TEST
    serverAddress.sin_port = htons(portNum);

    //bind server socket to port
    if (bind(serverSocket, (sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) {
        perror("Failed to bind socket to port (bind)!");
        exit(EXIT_FAILURE);
    }

    //listen for client connection
    if (listen(serverSocket, SOMAXCONN) == -1){
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

    char packet[15];
    read(clientSocket, packet, sizeof(packet));

    std::cout << "Received packet:" << std::endl;
    printPacket(packet, 10);

}