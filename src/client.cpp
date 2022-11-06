//
// Created through the efforts of Connor Kamrowski, Spencer Keith, and Jennifer Curry
//
//CHECK TODO flag in comments for unfinished code
//

#include <iostream>
#include <vector>
#include <string>
#include <iterator>
#include <fstream>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "packet.h"
#include <chrono>

#define FINAL_SEQUENCE_NUMBER -1


using namespace std;

//Function definitions
void writeFileToPacket(int sequenceNumber);

void sendPacket(int clientSocket, int sequenceNumber);

std::string userStringPrompt(std::string request);

int userIntegerPrompt(std::string request);

void executeSRProtocol(int clientSocket, sockaddr_in server_address);

//Variable definitions
int packet_size, timeout_interval, window_size, sequence_range, port_num;
std::string file_path; //path to file to be sent
Packet myPacket;

//Call to Main
int main() {

	//take user input
	std::cout << "Welcome to the scheduler. Provide the following information. \n" ;

	packet_size = userIntegerPrompt("Input packet size:");
	port_num = userIntegerPrompt("Input port number (9000-9999):");
	//timeout_interval = userIntegerPrompt("Input timeout interval:");
	//window_size = userIntegerPrompt("Input window size:");
	sequence_range = userIntegerPrompt("Input sequence range:");
	//TODO
	//situationalErrorsPrompt();
	file_path = userStringPrompt("Input path of file to transfer:");

	//connect socket for sending packets, listening for acks
	//	use ports 9000-9999
    struct sockaddr_in server_address = {0};
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);//SOCK_STREAM for TCP
    if(clientSocket == -1) {
        perror("Failed to create client socket");
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_num);
	
	//for now, just run the protocol on a single file
	executeSRProtocol(clientSocket, server_address);

}


//Function implementation

/*
 * ExecuteSRProtocol implements the actual selective repeat protocol
 */
void executeSRProtocol(int clientSocket, sockaddr_in server_address) {

	//create time_points for start and end
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
	//log the starting time
    startTime = std::chrono::system_clock::now();

	//iterator to count the number of packets sent
    int iterator = 0;
	int client_fd;
	
	//connect to the server
    if ((client_fd = connect(clientSocket, (struct sockaddr*)&server_address, sizeof(server_address))) < 0) {
        printf("\nConnection Failed \n");
        exit(0);
    }	
	
	//Don't take poseidon down, just keep this failsafe implemented in case something fails but it keeps running
	int FAILSAFE = 0;
    while(FAILSAFE < 1000000000) {
		FAILSAFE++;

        if(iterator >= sequence_range) {
            sendPacket(clientSocket, FINAL_SEQUENCE_NUMBER);
			cout << "Final sequence number reached." << std::endl;
            break;
        }

		//send the packet
		sendPacket(clientSocket, iterator);

        Packet myAck{};

        if(recv(clientSocket, &myAck, sizeof(myAck), MSG_DONTWAIT)) {
            std::cout << "Received ack #" << myAck.sequenceNumber << std::endl;
            iterator = myAck.sequenceNumber + 1;
        }

    }

	if(FAILSAFE >= 1000000000){
		std::cout << std::endl << "Program timed out (failsafe for Poseidon stopped the code)." << std::endl;
		exit(0);
	}
	
	std::cout << std::endl << std::endl << "Successfully Transmitted" << std::endl;

    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << std::endl << "Total execution time = " << elapsedSeconds.count() << std::endl;

    close(client_fd);

}

/*
 * writeFileToPacket gets the packet at a specified sequence number of the file
 *
 */
void writeFileToPacket(int sequenceNumber) {

    //create ifstream object
    std::ifstream fileInputStream;
    //open file at file_path in read and binary modes
    fileInputStream.open(file_path, std::ios_base::in | std::ios_base::binary);
    //navigate to section of file beginning at (sequenceNumber * packet_size) offset from beginning
    fileInputStream.seekg(sequenceNumber * packet_size, std::ios_base::beg);

    //create char array for file contents
    char contents[packet_size];
    //read file contents into array of amount packet_size
    fileInputStream.read(contents, packet_size);

    //set global packet struct sequence number
    myPacket.sequenceNumber = sequenceNumber;
    if(myPacket.sequenceNumber != FINAL_SEQUENCE_NUMBER) {
        //copy the contents of the array to the global packet struct char vector
        for (int i = 0; i < packet_size; i++) {
            myPacket.contents[i] = contents[i];
        }
    } else {
        for (int i = 0; i < packet_size; i++) {
            myPacket.contents[i] = '\0';
        }
    }

    fileInputStream.close();

}

/*
 * sendPacket sends the packet to the server
 *
 */
void sendPacket(int clientSocket, int sequenceNumber) {

    writeFileToPacket(sequenceNumber);

    myPacket.valid = true;

	int result = send(clientSocket, &myPacket, sizeof(myPacket), 0);
	
	cout << "result of send:" << result << std::endl;

	std::cout << "Sent Packet #" << myPacket.sequenceNumber << ": [ ";
	for(int i = 0; i < packet_size; i++) {
		std::cout << myPacket.contents[i];
	}
	std::cout << " ]" << std::endl;

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

