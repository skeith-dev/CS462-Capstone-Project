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

		int packetArrSize = (int) (packet_size + sizeof(int) + sizeof(bool) + 10); //TODO -> + sizeof(CHECKSUM)
		char packet[packetArrSize];

		if(recv(serverSocket, &packet, sizeof(packet), MSG_DONTWAIT) > 0) {//TODO - sizeof(packet) could just be packetArrSize?

			std::cout << "Packet: " << packet << std::endl;
		
			std::string packet_str(packet);
			int packet_sequence_number = std::stoi(packet_str.substr(0, sizeof(int)));
			bool packet_valid = std::stoi(packet_str.substr(sizeof(int), sizeof(bool)));
			//TODO - this is where checksum would be taken out
			int sum = 0;
			string binary = "";
			cout << packetSize << endl;
			for(int i = 0; i < packetSize; i++){
				if(i < packetSize){
					int val = int(a[i]);
					cout << val << endl;
					while(val > 0){
						if(val % 2){
							binary.push_back('1');
						} else {
							binary.push_back('0');
						}
						val /= 2;
					}
					sum += stoi(binary);
					cout << sum << endl;
					binary = "";
					cout << i << endl;
				}
			}
	
			string reCheck = "";
			cout << "we get here?" << endl;
			for(int j = packetSize; j < packetSize + 6; j++){
				cout << "a[" << j << "} = " << a[j] << endl;
				reCheck += a[j];
			}
	
			int check = stoi(reCheck);
	
			if(check = sum){
				cout << "Checksum OK" << endl;
			} else {
				cout << "Checksum failed" << endl;
			}
			std::string packet_contents = packet_str.substr((int) (sizeof(int) + sizeof(bool)));//this should give the packet contents

			//TODO - REMOVE AFTER TESTING
			std::cout << "Complete packet: " << packet << std::endl;
			std::cout << "Packet seq#: " << packet_sequence_number << std::endl;
			std::cout << "packet valid: " << packet_valid << std::endl;
			std::cout << "packet contents: " << packet_contents << std::endl;
			exit(0);

			//TODO - remove FINAL_SEQUENCE_NUMBER
			if(packet_sequence_number == FINAL_SEQUENCE_NUMBER) {
				//TODO - this should be a sendAck function. Add this to packetIO.cpp
				int ack_packet_size = (int) (sizeof(int) + sizeof(bool) + 1);
				char ack_packet[ack_packet_size];
				ack_packet[-1] = 6;//ACK value
				sendPacket(serverSocket, ack_packet, FINAL_SEQUENCE_NUMBER, 1);
				//TODO - reimplement this -> sendAck(serverSocket, FINAL_SEQUENCE_NUMBER);
				break;
			}
			//TODO - this doesn't work with sequence range logic, fix this. What if packet_sequence_number==0, and window[0]==13?
			if(packet_sequence_number < SR_window[0]) {
				std::cout << "Packet " << packet_sequence_number << " lost the ack, resend" << std::endl;

				//TODO - this should be a sendAck function. Add this to packetIO.cpp
				int ack_packet_size = (int) (sizeof(int) + sizeof(bool) + 1);
				char ack_packet[ack_packet_size];
				ack_packet[-1] = 6;//ACK value //TODO - do we want ack values? No real need for it, but it uses the space
				sendPacket(serverSocket, ack_packet, packet_sequence_number, 1);
				//TODO - reimplement this -> sendAck(serverSocket, packet_sequence_number);
				continue;
			}

            std::cout << "Packet " << packet_sequence_number << " recieved" << std::endl;

			bool EOW_found = false;//was the End of Window (-1) found?
			
			//if the received packet is the final packet
			if (packet_sequence_number == FINAL_SEQUENCE_NUMBER) {
				int finalSequenceNumber;
				int pos=0;
				//then find the sequence number of the final packet (STORED IN packet_contents)
				//TODO_MERGE - after merge with spencer, remove the period. Use end of packet stuff instead (packet length?)
				while (true) {
					if (packet_contents[pos] = '.') {//TODO - change to EOT??? see packetIO.cpp
						break;
					}
					pos++;
				}
				finalSequenceNumber = std::stoi(packet_contents.substr(0, pos));
				
				//TODO - remove prints after test
				std::cout << "Final string pos:" << pos << std::endl;
				std::cout << "Final string:" << packet_contents.substr(0, pos) << std::endl;
				std::cout << "Final sequence number:" << finalSequenceNumber << std::endl;
				exit(0);
				
				packet_sequence_number = finalSequenceNumber;
			}
			
			//find the index of the received packet, and send the ack
			for (int i = 0; i < slidingWindowSize; i++) {
				if (EOW_found) {
					//TODO - does this work...?
					SR_window[i] = FINAL_SEQUENCE_NUMBER;
				} else if (SR_window[i] == packet_sequence_number) {
					//TODO - this should be a sendAck function. Add this to packetIO.cpp
					int ack_packet_size = (int) (sizeof(int) + sizeof(bool) + 1);
					char ack_packet[ack_packet_size];
					ack_packet[-1] = 6;//ACK value
					sendPacket(serverSocket, ack_packet, packet_sequence_number, 1);

					//TODO - reimplement this -> sendAck(serverSocket, packet_sequence_number);
					received_packet_contents[i] = packet_contents;
					iterator++;
					if (packet_sequence_number == FINAL_SEQUENCE_NUMBER) {
						SR_window[i] = FINAL_SEQUENCE_NUMBER;
						EOW_found = true;
					} else {
						SR_window[i] = -2;
						break;
					}
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
					}// else if () {//if -1, it is the last
						//TODO - I don't remember what's going on here, but it's unfinished logic somehow
						// It's part of the rework to remove FINAL_SEQUENCE_NUMBER. Remove FINAL_SEQUENCE_NUMBER
//					}
					received_packet_contents[slidingWindowSize-1] = 'NULL';
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
