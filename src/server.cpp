#include <iostream>
#include <string>
#include <fstream>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "packet.h"


int portNum; //port number of the target server
int packetSize; //specified size of packets to be sent
int slidingWindowSize;  //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
std::string filePath;


using namespace std;

void main() {
	
	portNum = portNumPrompt();
	packetSize = packetSizePrompt();
	filePath = filePathPrompt();
	
	//create a socket
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1){
		cerr << "No socket created";
		return -1;
	}
	
	//bind the socket to an ip address and port
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNum);
	
	
	if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1){
		cerr << "can't bind to port";
		return -2;
	}
	//tell the socket to listen
	if (listen(serverSocket, SOMAXCONN) == -1){
		cerr << "Can't listen";
		return -3;
	}
	
	//wait for connection
	sockaddr_in client;
	soclen_t slientSize = sizeof(client);
	int clientSocket = accept(listening, (socketaddr*)&client, &clientSize);
	if(slientSocket == -1){
		cerr << "Problem with client connecting";
		return -4;
	}
	
	//close listening socket
	close(serverSocket);
	
	//start recieving
	executeSAW_GBNProtocol(serverSocket, clientAddress, clientSize);
	
	
	//close socket
	close(clientSocket);
	
}

std::string filePathPrompt() {

    std::cout << "What is the filepath of the file you wish to write to:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return responseString;

}

int portNumPrompt() {

    std::cout << "What is the port number of the target server:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString);

}

int packetSizePrompt() {

    std::cout << "Size of packets:" << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return std::stoi(responseString);

}

void writePacketToFile(bool append, const std::string& message) {

    std::ofstream fileOutputStream;
    if(append) {
        fileOutputStream.open(filePath, std::ios_base::app);
    } else {
        fileOutputStream.open(filePath);
    }
    fileOutputStream << message;

    fileOutputStream.close();

}

void executeSAW_GBNProtocol(int serverSocket, sockaddr_in clientAddress, int clientSize) {

    iterator = 0;
    while(true) {

        Packet myPacket{};

        if(recvfrom(serverSocket, &myPacket, sizeof(myPacket), MSG_DONTWAIT, (struct sockaddr*)&clientAddress, reinterpret_cast<socklen_t *>(&clientSize)) != -1) {

            if(myPacket.sequenceNumber == FINAL_SEQUENCE_NUMBER) {
                break;
            }

            std::cout << "Received packet #" << myPacket.sequenceNumber << "! [ ";
            for(int i = 0; i < packetSize; i++) {
                std::cout << myPacket.contents[i];
            }
            std::cout << " ]" << std::endl;

            if(myPacket.valid && myPacket.sequenceNumber == iterator) {
                //sendAck(serverSocket, clientAddress, iterator);
                writePacketToFile(true, myPacket.contents);
                iterator++;
            } else {
                std::cout << "Received packet is corrupted!" << std::endl;
            }

        }

    }

}
