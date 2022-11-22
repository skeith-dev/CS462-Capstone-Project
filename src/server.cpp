//
// Created by Spencer Keith on 11/18/22.
//

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include "prompts.cpp"
#include "fileIO.cpp"
#include "packetIO.cpp"
#include <bits/stdc++.h>

#define FINAL_SEQUENCE_NUMBER -1


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function declarations                          //*****//*****//*****//

void stopAndWaitProtocol();

void selectiveRepeatProtocol(int serverSocket, int clientSize);


int packet_size, slidingWindowSize;
std::string filePath;

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
        int portNum = 9090;//userIntPrompt("What is the port number of the target server:", 0, 9999);
        serverAddress.sin_port = htons(portNum);
        if (bind(serverSocket, (sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) {
            perror("Failed to bind socket to port (bind)!");
            exit(EXIT_FAILURE);
        }
        //0 for S&W, 1 for SR
        int protocolType = 1;//userIntPrompt("Type of protocol, S&W (0) or SR (1):", 0, 1);
        //specified size of packets to be sent
        packet_size = 30;//userIntPrompt("Size of packets (must be same as sender):", 1, INT_MAX);
        //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8
        if(protocolType > 0) {
            slidingWindowSize = 10;//userIntPrompt("Size of sliding window:", 1, MAX_INPUT);
        }
        //path of file to be sent
        filePath = "output.txt";//userStringPrompt("What is the filepath of the file you wish to write TO:");

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
				std::cout << "TODO" << std::endl;
                //executeSAW_GBNProtocol(serverSocket, clientAddress, packet_size);
                break;
            case 1:
                std::cout << std::endl << "Executing Selective Repeat protocol..." << std::endl << std::endl;
                selectiveRepeatProtocol(clientSocket, clientSize);
                break;
            default:
                break;
        }

        quit = userBoolPrompt("Would you like to exit (1), or perform another file transfer (0):");
    } while(!quit);

	//TODO - do we have to close sockets? I don't know

}


void selectiveRepeatProtocol(int serverSocket, int clientSize) {

    int iterator = 0;
	
	int FAILSAFE = 0;
	
	int SR_window[slidingWindowSize];
	
	std::string received_packet_contents[packet_size];
	
	//populate SR_window with sequence numbers
	for (int i = 0; i < slidingWindowSize; i++)
		SR_window[i] = i;

	int last_packet_num = SR_window[-1];
	
    while(FAILSAFE < 1000000000) {
		FAILSAFE++;

		int packetArrSize = (int) (packet_size + sizeof(int) + sizeof(bool)); //TODO -> + sizeof(CHECKSUM)
		char packet[packetArrSize];

		if(recv(serverSocket, &packet, sizeof(packet), MSG_DONTWAIT) > 0) {//TODO - sizeof(packet) could just be packetArrSize?

			int packet_sequence_number = 0;
			bool packet_valid = int(packet[sizeof(int)]);
			std::string packet_contents;

			//this could be a function (probably unecessary, but it comes up in different files)
			//get the sequence number of the packet
			if (packet[0]==0 && packet[1]==1 && packet[2]==1 && packet[3]==0) {//final sequence num reached
				packet_sequence_number = -1;
			} else {//TODO - this is a rudimentary solution for the end of sequence
				for (int i=0; i < sizeof(int); i++) {
					packet_sequence_number += packet[i];
				}				
			}
			int length = (int) (sizeof(int) + sizeof(bool) + packet_size);
			for(int i = sizeof(int)+1; i < length; i++) {
				packet_contents = packet_contents + packet[i];
			}

			//TODO - remove FINAL_SEQUENCE_NUMBER
			if(packet_sequence_number == FINAL_SEQUENCE_NUMBER) {
				break;
			}
			//TODO - this doesn't work with sequence range logic, fix this. What if packet_sequence_number==0, and window[0]==13?
			if(packet_sequence_number < SR_window[0]) {
				std::cout << "Packet " << packet_sequence_number << " lost the ack, resend" << std::endl;

				//TODO - this should be a sendAck function. Add this to packetIO.cpp
				int ack_packet_size = (int) (sizeof(int) + sizeof(bool) + 1);
				char ack_packet[ack_packet_size];
				ack_packet[sizeof(int)+sizeof(bool)+1] = 6;//ACK value

				//TODO - move everything here until sendAck() into sendAck(), it comes up multiple times
				char packet_sequence_number_bytes[sizeof(int)];					
				int temp_seq_num = packet_sequence_number;
				
				for (int i=sizeof(int)-1; i >= 0; i--) {
					if(temp_seq_num - 127 > 0) {
						packet_sequence_number_bytes[i] = 127;
						temp_seq_num = temp_seq_num - 127;
					} else if (temp_seq_num - 127 < 0){
						packet_sequence_number_bytes[i] = temp_seq_num;
						temp_seq_num = 0;
					} else {
						packet_sequence_number_bytes[i] = 0;
					}
				}
				//write seqNum to ack
				for(int i=0; i<sizeof(int); i++) {
					ack_packet[i] = packet_sequence_number_bytes[i];
				}
				sendAck(serverSocket, ack_packet, packet_sequence_number);

				continue;
				//TODO - reimplement this -> sendAck(serverSocket, packet_sequence_number);
			}

            std::cout << "Packet " << packet_sequence_number << " recieved" << std::endl;
						
			//find the index of the received packet, and send the ack
			for (int i = 0; i < slidingWindowSize; i++) {
				if (SR_window[i] == packet_sequence_number) {
					//TODO - this should be a sendAck function. Add this to packetIO.cpp
					int ack_packet_size = (int) (sizeof(int) + sizeof(bool) + 1);
					char ack_packet[ack_packet_size];
					ack_packet[ack_packet_size-1] = 6;//ACK value
					
					//TODO - move everything here until sendAck() into sendAck(), it comes up multiple times
					char packet_sequence_number_bytes[sizeof(int)];					
					int temp_seq_num = packet_sequence_number;
					
					for (int i=sizeof(int)-1; i >= 0; i--) {
						if(temp_seq_num - 127 > 0) {
							packet_sequence_number_bytes[i] = 127;
							temp_seq_num = temp_seq_num - 127;
						} else if (temp_seq_num - 127 < 0){
							packet_sequence_number_bytes[i] = temp_seq_num;
							temp_seq_num = 0;
						} else {
							packet_sequence_number_bytes[i] = 0;
						}
					}
					//write seqNum to ack
					for(int i=0; i<sizeof(int); i++) {
						ack_packet[i] = packet_sequence_number_bytes[i];
					}

					sendAck(serverSocket, ack_packet, packet_sequence_number);

					//TODO - reimplement this -> sendAck(serverSocket, packet_sequence_number);
					received_packet_contents[i] = packet_contents;
					iterator++;
					SR_window[i] = -2;
					break;
				}
			}

			//check if the window needs to move
			if (SR_window[0] == -2) {
				while (SR_window[0] == -2) {
				//write the information to the file
				
				//TODO - is this working?
				writePacketToFile(true, received_packet_contents[0], filePath);
				//OPTIMIZE - while and for loop could be separated
					for(int i=0; i<slidingWindowSize-1; i++){
						SR_window[i] = SR_window[i+1];
						received_packet_contents[i] = received_packet_contents[i+1];
					}
					//If -2, it was received
					if (SR_window[slidingWindowSize-1] == -2) {
						SR_window[slidingWindowSize-1] = SR_window[slidingWindowSize-1]+1;
					}
					received_packet_contents[slidingWindowSize-1] = 'NULL';
				}
				//print the window
				std::cout << "receiver window: [";
				for (int i = 0; i < slidingWindowSize; i++)
					std::cout << SR_window[i] << ", ";
				std::cout << "]" << std::endl;
			} else if (SR_window[0] == FINAL_SEQUENCE_NUMBER) {//check if done
				break;
			}
		}

    }
	
	std::cout << "Last packet seq# received: " << iterator - 1 << std::endl;
	//std::cout << ":" << std::endl << "Successfully received file." << std::endl;
	close(serverSocket);

}