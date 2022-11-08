#include <iostream>
#include <string>
#include <fstream>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "packet.h"

#define FINAL_SEQUENCE_NUMBER -1


int portNum; //port number of the target server
int packetSize; //specified size of packets to be sent
int slidingWindowSize;  //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
std::string filePath;

std::string filePathPrompt();
int portNumPrompt();
int packetSizePrompt();
void writePacketToFile(bool append, const std::string& message);
void executeSRProtocol(int serverSocket, int clientSize);
void sendAck(int serverSocket, int sequenceNumber);
std::string userStringPrompt(std::string request);
int userIntegerPrompt(std::string request);



using namespace std;

int main() {
	
	std::cout << "Welcome to the scheduler. Provide the following information. \n" ;

	portNum = userIntegerPrompt("Input port number (9000-9999):");
	filePath = userStringPrompt("Enter path for file to write to:");
	std::cout<<std::endl;
	
	//create a socket
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1){
		cerr << "No socket created";
		return -1;
	}
	
    int opt = 1;
    // Forcefully attaching socket to the port
    if (setsockopt(serverSocket, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
	
    struct sockaddr_in serverAddress = {0};
	
	//bind the socket to an ip address and port
	serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; //TEST
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
	socklen_t clientSize = sizeof(client);
	int clientSocket = accept(serverSocket, (sockaddr*)&client, &clientSize);
	if(clientSocket == -1){
		cerr << "Problem with client connecting";
		return -4;
	}
	
	//close listening socket
	close(serverSocket);
	
	//start recieving
	executeSRProtocol(clientSocket, clientSize);
	
	
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

void executeSRProtocol(int serverSocket, int clientSize) {

    int iterator = 0;
	
	int FAILSAFE = 0;
    while(FAILSAFE < 1000000000) {
		FAILSAFE++;

        Packet myPacket{};

		if(recv(serverSocket, &myPacket, sizeof(myPacket), MSG_DONTWAIT) > 0) {
			
            if(myPacket.sequenceNumber == FINAL_SEQUENCE_NUMBER) {
                break;
            }

            std::cout << "Packet " << myPacket.sequenceNumber << " recieved" << std::endl;
            /*for(int i = 0; i < packetSize; i++) {
                std::cout << myPacket.contents[i];
            }
            std::cout << " ]" << std::endl;*/

            if(myPacket.sequenceNumber == iterator) {
                sendAck(serverSocket, iterator);
                writePacketToFile(true, myPacket.contents);
                iterator++;
            } else {
                std::cout << "Received packet is corrupted!" << std::endl;
            }
			
        }

    }
	
	//std::cout << ":" << std::endl << "Successfully received file." << std::endl;
	close(serverSocket);

}

void sendAck(int serverSocket, int sequenceNumber) {

    Packet myAck{};
    myAck.sequenceNumber = sequenceNumber;
    myAck.valid = true;
	int result = send(serverSocket, &myAck, sizeof(myAck), 0);
	//std::cout << "Result of send: " << result << std::endl; //REMOVE, THESE ARE FOR DEBUGGING
    std::cout << "Ack " << myAck.sequenceNumber << " sent" << std::endl;

}

/*
 * filePathPrompt takes the path to the file (to be sent) and saves it as a string
 */
std::string userStringPrompt(std::string request) {

	std::cout << request << std::endl;

	std::string responseString;
	std::getline(std::cin, responseString);

	return responseString;

}

/*
 * userIntegerPrompt handles any user input where the returned value is an integer
 */
int userIntegerPrompt(std::string request) {

	std::cout << request << std::endl;

	std::string responseString;
	std::getline(std::cin, responseString);

	return std::stoi(responseString);

}
