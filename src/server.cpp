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
int sequence_range; // TODO - explain this
std::string filePath;

std::string filePathPrompt();
int portNumPrompt();
int packetSizePrompt();
void writePacketToFile(bool append, const std::string& message);
void executeSRProtocol(int serverSocket, int clientSize);
void sendAck(int serverSocket, int sequenceNumber);
std::string userStringPrompt(std::string request);
int userIntegerPrompt(std::string request, bool restricted, int min, int max);



using namespace std;

int main() {
	
	std::cout << "Welcome to the scheduler. Provide the following information. \n" ;

	//TODO - remove the hard coding here
	portNum = 9090;//userIntegerPrompt("Input port number (9000-9999):", true, 9000, 9999);
	filePath = "output.txt";//userStringPrompt("Enter path for file to write to:");
	sequence_range = 30;//userIntegerPrompt("Input window size:", true, 1, 10000);
	slidingWindowSize = 15;//userIntegerPrompt("Input window size:", true, 1, sequence_range/2);
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
	
	int SR_window[slidingWindowSize];
	Packet packet_contents[slidingWindowSize];
	
	//populate SR_window with sequence numbers
	for (int i = 0; i < slidingWindowSize; i++)
		SR_window[i] = i;
	
    while(FAILSAFE < 1000000000) {
		FAILSAFE++;

        Packet myPacket{};

		if(recv(serverSocket, &myPacket, sizeof(myPacket), MSG_DONTWAIT) > 0) {
			if(myPacket.sequenceNumber == FINAL_SEQUENCE_NUMBER) {
				sendAck(serverSocket, FINAL_SEQUENCE_NUMBER);
				break;
			}
			if(myPacket.sequenceNumber < SR_window[0]) {
				//std::cout << "Packet " << myPacket.sequenceNumber << " lost the ack, resend" << std::endl;
				//sendAck(serverSocket, myPacket.sequenceNumber);
				continue;
			}

            std::cout << "Packet " << myPacket.sequenceNumber << " recieved" << std::endl;
			//PRINT CONTENTS(For debugging)
			//HARD_DEBUG
/*            for(int i = 0; i < sizeof(myPacket.contents); i++) {
                std::cout << myPacket.contents[i];
            }
            std::cout << " ]" << std::endl << std::endl;
*/
			//bool inArray = 0;//STRIKE
			for (int i = 0; i < slidingWindowSize; i++) {
				if (SR_window[i] == myPacket.sequenceNumber) {
					sendAck(serverSocket, myPacket.sequenceNumber);
					packet_contents[i] = myPacket;
					iterator++;
					if (myPacket.sequenceNumber == FINAL_SEQUENCE_NUMBER) {
						SR_window[i] == FINAL_SEQUENCE_NUMBER;
					} else {
						SR_window[i] = -2;
					}
					break;
				}
			}

			//check if the window needs to move
			if (SR_window[0] == -2) {
				while (SR_window[0] == -2) {
				//write the information to the file
				writePacketToFile(true, packet_contents[0].contents);
				//OPTIMIZE - while and for loop could be separated
					for(int i=0; i<slidingWindowSize-1; i++){
						SR_window[i] = SR_window[i+1];
						packet_contents[i] = packet_contents[i+1];
					}
					SR_window[slidingWindowSize-1] = SR_window[slidingWindowSize-1] % sequence_range;
					Packet tempPacket{};
					packet_contents[slidingWindowSize-1] = tempPacket;
				}
				//print the window
				for (int i = 0; i < slidingWindowSize; i++)
					std::cout << SR_window[i] << ", ";
				std::cout << std::endl;
			} else if (SR_window[0] == FINAL_SEQUENCE_NUMBER) {//check if done
				break;
			}
		}

    }
	
	std::cout << "Last packet seq# received: " << iterator - 1 << std::endl;
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
int userIntegerPrompt(std::string request, bool restricted, int min, int max) {

	std::string responseString;
	std::cout << request << std::endl;
	std::getline(std::cin, responseString);		
	if (restricted) {
		while (std::stoi(responseString) < min || std::stoi(responseString) > max) {
			std::cout << std::endl << "Input must be between " << min << " and " << max << ". ";
			std::cout << request << std::endl;
			std::getline(std::cin, responseString);
		}
	}

	return std::stoi(responseString);

}
