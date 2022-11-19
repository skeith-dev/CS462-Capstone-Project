//
// Created through the efforts of Connor Kamrowski, Spencer Keith, and Jennifer Curry
//
//CHECK TODO tag
// TODO LIST - things that need to be implemented or changed
// timeout_interval, window_size, 'error' simulator (packet loss), accepted/sent array memory too high
//
//CHECK OPTIMIZE tag
// OPTIMIZE LIST - things that are inefficient, but work. To make the program better we can look at these
//
//

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
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

int sendPacket(int clientSocket, int sequenceNumber);

std::string userStringPrompt(std::string request);

int userIntegerPrompt(std::string request, bool restricted, int min, int max);

void executeSRProtocol(int clientSocket, sockaddr_in server_address);

int openFile();


//Variable definitions
int packet_size, timeout_interval, window_size, sequence_range, port_num, num_packets;
std::string file_path; //path to file to be sent
Packet myPacket;

//Call to Main
int main() {

	//take user input
	std::cout << "Welcome to the scheduler. Provide the following information. \n" ;

	//TODO - remove the hard coding here
	packet_size = 30;//userIntegerPrompt("Input desired packet size (bytes):", true, 1, 23333);
	port_num = 9090;//userIntegerPrompt("Input port number (9000-9999):", true, 9000, 9999);
	timeout_interval = 400000000;//userIntegerPrompt("Input timeout interval:", true, 10, 4000000);
	sequence_range = 3000;//userIntegerPrompt("Input sequence range:", true, 1, 9999999);
	window_size = 15;//userIntegerPrompt("Input window size:", true, 1, int(sequence_range/2));//Window size should never be greater than 1/2sr
	//TODO
	//situationalErrorsPrompt();
	file_path = userStringPrompt("Input path of file to transfer:");
	std::cout<<std::endl;

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

	//iterator to count the number of packets sent
    int iterator = 0;
    int window_position = 0;
	int client_fd;

	//create time_points for start and end
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
	//log the starting time
    startTime = std::chrono::system_clock::now();

	//connect to the server
    if ((client_fd = connect(clientSocket, (struct sockaddr*)&server_address, sizeof(server_address))) < 0) {
        printf("\nConnection Failed \n");
        exit(0);
    }

	num_packets = openFile();
	std::cout << std::endl << "Num packets:" << num_packets << std::endl;
	//TODO - memory efficiency, make this match the window somehow
	
	int SR_window[window_size];
	bool sent[window_size] = { 0 }; //OPTIMIZE - this and next array could be simplified
	std::chrono::_V2::system_clock::time_point sent_times[window_size];
	
	//populate SR_window with sequence numbers
	for (int i = 0; i < window_size; i++) {
		if (i >= num_packets) {
			SR_window[i] = -1;
		} else {
			SR_window[i] = i;
		}
	}

	double sent_data;
	//TODO - sequence range works with a loop around -> 0, 1, 2, 3, 0, 1, 2, 3 -> UNTIL DONE!

	//Don't take poseidon down, just keep this failsafe implemented in case something fails but it keeps running
	int FAILSAFE = 0;
    while(FAILSAFE < 1000000000) {
		FAILSAFE++;

		//send the packet
		//TODO - infinite sending/rec with unreal packets. Maybe a check here - if > num_packets, break
		if (SR_window[window_position] == -1) {
			if(window_position == 0) {
				sendPacket(clientSocket, FINAL_SEQUENCE_NUMBER);
			}
			window_position = 0;
		}
		if (sent[window_position] == 0){
			//the packet hasn't been sent yet, send it
			sent_data += sendPacket(clientSocket, SR_window[window_position]);
			sent[window_position] = 1;
			sent_times[window_position] = std::chrono::system_clock::now();			
			
		} else if (SR_window[window_position] != -2 && (std::chrono::system_clock::now() - sent_times[window_position]).count() > timeout_interval) {
			//the packet timed out, send again
			std::cout << "** Packet " << SR_window[window_position] << " timed out. Sending again **" << std::endl;
			sent_data += sendPacket(clientSocket, SR_window[window_position]);
			sent_times[window_position] = std::chrono::system_clock::now();

		}
		//the packet is currently being sent, or it was already accepted. Either way, check the next.
		window_position = (window_position+1) % window_size;

        Packet myAck{};

		//check for acks
        if(recv(clientSocket, &myAck, sizeof(myAck), MSG_DONTWAIT) > 0) {
			if (myAck.sequenceNumber == FINAL_SEQUENCE_NUMBER) {
				if (iterator == num_packets) {break;}
				else {continue;}
			}
            std::cout << "Received ack #" << myAck.sequenceNumber << std::endl;
            iterator++;
			//OPTIMIZE
			for(int index = 0; index < window_size; index++) {
				if(myAck.sequenceNumber == SR_window[index]) {
					SR_window[index] = -2;
					break;
				}
			}
				
			//check for window (does it need to move?)
			//below logic => if received at the window's start is -1, then move window up until it isn't
			if (SR_window[0] == -2) {
				while (SR_window[0] == -2) {
				//OPTIMIZE - while and for loop could be separated
					for(int i=0; i<window_size-1; i++){
						SR_window[i] = SR_window[i+1];
						sent[i] = sent[i+1];
					}
					if(SR_window[window_size-2] == -1) {
						SR_window[window_size-1] = -1;
					} else {
						//TODO - test this logic, sequence_range
						SR_window[window_size-1] = (SR_window[window_size-2] + 1) % sequence_range;
					}
					sent[window_size-1] = 0;
					//FIX - the window position has to follow the moving window positions
					window_position = (window_position-1) % window_size;
				}
				// Print the (new) current window
				std::cout << "Current window: [";
				for (int i = 0; i < window_size-1; i++) 
					std::cout << SR_window[i] << ", ";
				std::cout << SR_window[window_size-1] << "]" << std::endl;
			}
			if (SR_window[0] == -1 && myAck.sequenceNumber == -1) {
				break;
			}

        }
		
		
		//end of while loop
    }

	if(FAILSAFE >= 1000000000){
		std::cout << std::endl << "Program timed out (failsafe for Poseidon stopped the code)." << std::endl;
		exit(0);
	}
	
	std::cout << ":" << std::endl << ":" << std::endl << "Session successfully terminated" << std::endl;

    endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = endTime - startTime;
    std::cout << std::endl << "Number of packets sent: " << iterator << std::endl;
    std::cout << "Total execution time: " << elapsedSeconds.count() << std::endl;
    std::cout << "Total throughput (bps): " << sent_data/elapsedSeconds.count() << std::endl;

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
            myPacket.contents[0] = '\0';
        }
    }

    fileInputStream.close();

}

/*
 * sendPacket sends the packet to the server
 *
 */
int sendPacket(int clientSocket, int sequenceNumber) {

    writeFileToPacket(sequenceNumber);

    myPacket.valid = true;

	int result = send(clientSocket, &myPacket, sizeof(myPacket), 0);
	
	//cout << "result of send:" << result << std::endl;

	if (myPacket.sequenceNumber != -1) {
		std::cout << "Sent Packet #" << myPacket.sequenceNumber << std::endl;
	}
	return result;
/*	std::cout << "Sent Packet #" << myPacket.sequenceNumber << ": [ ";
	for(int i = 0; i < packet_size; i++) {
		std::cout << myPacket.contents[i];
	}
	std::cout << " ]" << std::endl;
*/
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

/*
 * openFile calculates the number of packets that will be sent
 */
int openFile() {

    std::ifstream fileInputStream;

    fileInputStream.open(file_path, std::ios_base::in | std::ios_base::binary);
    if (fileInputStream.fail()) {
        throw std::fstream::failure("Failed while opening file " + file_path);
    }

    fileInputStream.seekg(0, fileInputStream.end);
    int fileSize = (int) fileInputStream.tellg();
    int fileSizeRangeOfSequenceNumbers = fileSize / packet_size + 1;
	
	if (fileSizeRangeOfSequenceNumbers < 1) {
		fileSizeRangeOfSequenceNumbers = 1;
	} else if ((fileSize % packet_size) > 0) fileSizeRangeOfSequenceNumbers++;

    std::cout << std::endl << "File to deliver: " << file_path << std::endl << "File size: " << fileSize << " bytes" << std::endl << std::endl;
	//"good practice would be enter packet size in mb, bytes, etc"

    fileInputStream.close();
	
	return fileSizeRangeOfSequenceNumbers;

}
