//
// Created by Spencer Keith on 11/18/22.
//


//REMOVE THIS BEFORE SUBMISSION
//TODO LIST
/*
 * BIG CHANGES
 * 	Things that have to be changed before we submit, or are huge problems in the code
 *
 * *** REMOVE hard coded inputs. This is for ease of testing. Look for Prompt(
 *
 */

/*
 * MEDIUM CHANGES
 * 	Things that need to be changed, but aren't the priority
 *
 */

/*
 * SMALL CHANGES
 * 	Things that we could change if we have time. Just look around for //TODO or //OPTIMIZE
 * 	I ended up leaving notes when I did something poorly, with hopes that it could be fixed later
 *
 */



#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include "prompts.cpp"
#include "fileIO.cpp"
#include "packetIO.cpp"
#include <bits/stdc++.h>

#define FINAL_SEQUENCE_NUMBER -1


//*****//*****//*****//*****//*****//*****//*****//*****//*****//*****//
//Function declarations                          //*****//*****//*****//

void stopAndWaitProtocol();

void selectiveRepeatProtocol(int clientSocket, sockaddr_in server_address);
int packet_size, timeout_interval, window_size, sequence_range, port_num, num_packets, fileSize, fileSizeRangeOfSeqNums, slidingWindowSize;
std::string filePath;

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
        int portNum = 9090;//userIntPrompt("What is the port number of the target server:", 0, 9999);
        serverAddress.sin_port = htons(portNum);
//TODO        std::cout << std::endl << "Connecting to server..." << std::endl << std::endl;
//        if (connect(clientSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0) {
//            std::cout << "Connection to server failed!" << std::endl;
//            exit(0);
//        }

        //0 for S&W, 1 for SR
        int protocolType = 1;//userIntPrompt("Type of protocol, S&W (0) or SR (1):", 0, 1);


        //specified size of packets to be sent
		//relates only to the contents, not the header. More bytes will be sent than this
        packet_size = 30;//userIntPrompt("Size of packets:", 1, INT_MAX);

        //user-specified (0+) or default (-1)
        timeout_interval = 99999;//userIntPrompt("Timeout interval, user-specified or ping calculated (-1):", -1, INT_MAX);
        //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8

		//TODO - he says not infinite, but the max could really be anything
		sequence_range = 100;//userIntPrompt("Sequence range: ", 2, 5000);

        if(protocolType > 0) {
            window_size = 10;//userIntPrompt("Size of sliding window:", 1, sequence_range/2);
        }
        //none (0), randomly generated (1), or user-specified (2)
        int situationalErrors = 0;//userIntPrompt("Situational errors; none (0), randomly generated (1), or user-specified (2):", 0, 2);
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
        filePath = userStringPrompt("What is the filepath of the file you wish to write FROM:");

        //size of file in bytes
        fileSize = openFile(filePath);
        //the range of sequence numbers necessary to send the whole file
		//TODO - rework this for true sequence numbers. The range required doesn't apply to SR, but we should need the var below
        fileSizeRangeOfSeqNums = fileSize / packet_size + fileSize % packet_size;
        //ex. (sliding window size = 3) [1, 2, 3] -> [2, 3, 4] -> [3, 4, 5], range = 5
//        std::stringstream ss;
//        ss << "Range of sequence numbers (" << fileSizeRangeOfSeqNums << " required to send entire file):";
//        int sequence_range = userIntPrompt(ss.str(), 1, fileSizeRangeOfSeqNums);
		//This had to be moved up, and there is no true max for sequence_range (but he didn't want infinite)

        switch (protocolType) {
            case 0:
                std::cout << std::endl << "Executing Stop & Wait protocol..." << std::endl << std::endl;
                //stopAndWaitProtocol(clientSocket, serverAddress);
                break;
            case 1:
                std::cout << std::endl << "Executing Selective Repeat protocol..." << std::endl << std::endl;
                selectiveRepeatProtocol(clientSocket, serverAddress);
                break;
            default:
                break;
        }

        quit = userBoolPrompt("Would you like to exit (1), or perform another file transfer (0):");
    } while (!quit);

    return 0;

}

/*
 * selectiveRepeatProtocol implements the actual selective repeat protocol
 */
void selectiveRepeatProtocol(int clientSocket, sockaddr_in server_address) {

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
    }// TODO - is this good to remove now?

	num_packets = openFile(filePath);
	std::cout << std::endl << "Num packets:" << num_packets << std::endl;
	
	int SR_window[window_size];
	bool sent[window_size] = { 0 }; //0==unsent,1==sent,2==received    ...and, -1==END???
	std::chrono::_V2::system_clock::time_point sent_times[window_size];
	
	//populate SR_window with sequence numbers
	for (int i = 0; i < window_size; i++) {
		//TODO - we can leave this, and just have the final packet include an EOF char, but we have to do that then
		//This relates to removing FINAL_SEQUENCE_NUMBER
		SR_window[i] = i;
//		if (i >= num_packets) {
//			SR_window[i] = -1;
//		} else ...
	}

	double sent_data;
	//TODO - sequence range works with a loop around -> 0, 1, 2, 3, 0, 1, 2, 3 -> UNTIL DONE!
	
	std::cout << std::endl << std::endl;

	//Don't take poseidon down, just keep this failsafe implemented in case something fails but it keeps running
	int FAILSAFE = 0;
    while(FAILSAFE < 1000000000) {
		FAILSAFE++;

		//TEMP - remove this
		std::cout << std::endl << std::endl;
		std::cout << "SR Window: [";
		for (int i = 0; i < window_size; i++) {
			std::cout << SR_window[i] << ", ";
		}
		std::cout << "]" << std::endl << std::endl << std::endl;

		//send the packet
		//TODO - infinite sending/rec with unreal packets. Maybe a check here - if > num_packets, break
		if (SR_window[window_position] == -1 && sent[window_position] == 0) {//TODO - this if isn't doing anything, remove...
			if(window_position == 0) {
				//TODO - remove packets! - do at all sendPacket() calls
				//sendPacket(clientSocket, SR_window[window_position], false);

				//TODO - sequence range loops, make writeFileToPacket work with that
				//	loop for sequence_range==4 -> [0, 1, 2, 3, 0, 1, 2, 3, 0, ...]
				
				//TODO - this should be sending FINAL_SEQUENCE_NUMBER. Make sure this actually sends that
				int packetArrSize = (int) (packet_size + sizeof(int) + sizeof(bool)); //TODO -> + sizeof(CHECKSUM)
				char packet[packetArrSize];
				writeFileToPacket(packet, filePath, fileSize, SR_window[window_position], iterator, packet_size, fileSizeRangeOfSeqNums);
				sendPacket(clientSocket, packet, SR_window[window_position], packet_size);
				
				//log that the packet was sent, and the time which it was sent
				sent[window_position] = 1;
				sent_times[window_position] = std::chrono::system_clock::now();
				
			} else {
				window_position = 0;
			}
		} else if (sent[window_position] == 0){
			//the packet hasn't been sent yet, send it
			//sent_data += sendPacket(clientSocket, SR_window[window_position], true);
			//TODO -> fix sent_data
			//TODO -> make the lines below a function maybe??
			int packetArrSize = (int) (packet_size + sizeof(int) + sizeof(bool)); //TODO -> + sizeof(CHECKSUM)
			char packet[packetArrSize];
			writeFileToPacket(packet, filePath, fileSize, SR_window[window_position], iterator, packet_size, fileSizeRangeOfSeqNums);
			sendPacket(clientSocket, packet, SR_window[window_position], packet_size);

			std::cout << std::endl << "Window position: " << window_position << std::endl;
			std::cout << "SR Window: [";
			for (int i = 0; i < window_size; i++) {
				std::cout << SR_window[i] << ", ";
			}
			std::cout << "]" << std::endl << std::endl << std::endl;
			sent[window_position] = 1;
			sent_times[window_position] = std::chrono::system_clock::now();
			
		} else if (SR_window[window_position] != -2 && (std::chrono::system_clock::now() - sent_times[window_position]).count() > timeout_interval) {
			//the packet timed out, send again
			std::cout << "** Packet " << SR_window[window_position] << " timed out. Sending again **" << std::endl;

			int packetArrSize = (int) (packet_size + sizeof(int) + sizeof(bool)); //TODO -> + sizeof(CHECKSUM)
			char packet[packetArrSize];
			writeFileToPacket(packet, filePath, fileSize, SR_window[window_position], iterator, packet_size, fileSizeRangeOfSeqNums);
			sendPacket(clientSocket, packet, SR_window[window_position], packet_size);

			sent_times[window_position] = std::chrono::system_clock::now();

		}
		//the packet is currently being sent, or it was already accepted. Either way, check the next.
		//i.e. -> increment window_position along, making sure to never go above window_size
		window_position = (window_position+1) % window_size;

		int packetArrSize = (int) (1 + sizeof(int) + sizeof(bool)); //TODO -> + sizeof(CHECKSUM)? Not for getting acks...
		char ack[packetArrSize];

		//check for acks
        if(recv(clientSocket, &ack, sizeof(ack), MSG_DONTWAIT) > 0) {
			//TODO - checksum changes here too
			int ack_sequence_number;
			bool ack_valid = int(ack[sizeof(int)]);

			//this could be a function (probably unecessary, but it comes up in different files)
			//get the sequence number of the ack
			for (int i=0; i < sizeof(int); i++) {
				ack_sequence_number += ack[i];
			}
			
			//TODO - remove this, *ONLY* process FINAL_SEQUENCE_NUMBER at window shifting, it's easier!
			if (ack_sequence_number == FINAL_SEQUENCE_NUMBER) {

				//TODO - does num_packets still work???
				//TODO - remove this now
				if (iterator == num_packets) {break;}
				else {continue;}
			}
            std::cout << "Received ack #" << ack_sequence_number << std::endl;
            iterator++;
			//OPTIMIZE
			//find the location of the acked packet, and signify that the packet was acked

			//TODO - error case, drop the ack here or accept it by running the for loop below (or does that happen in server?)
			//if (ack_valid) {} else {//print that ack was dropped intentionally}
			for(int index = 0; index < window_size; index++) {
				if(ack_sequence_number == SR_window[index]) {
					SR_window[index] = -2;
					break;
				}
			}
				
			//check for window (does it need to move?)
			//below logic => if received at the window's start is -2, then move window up until it isn't
			if (SR_window[0] == -2) {
				while (SR_window[0] == -2) {
				//OPTIMIZE - while and for loop could be separated
					for(int i=0; i<window_size-1; i++){
						SR_window[i] = SR_window[i+1];
						sent[i] = sent[i+1];
					}
					if(SR_window[window_size-2] == -1) {
						SR_window[window_size-1] = -1;
					} else if (SR_window[window_size-2] > sequence_range) {
						SR_window[window_size-1] = 0;
					} else {
						//TODO - test this logic, sequence_range
						SR_window[window_size-1] = (SR_window[window_size-2] + 1) % sequence_range;
					}
					sent[window_size-1] = 0;
					//FIX - the window position has to follow the moving window positions
					window_position = window_position-1;
					if (window_position < 0) window_position = window_size-1;
				}
				// Print the (new) current window
				std::cout << "Current window: [";
				for (int i = 0; i < window_size-1; i++) 
					std::cout << SR_window[i] << ", ";
				std::cout << SR_window[window_size-1] << "]" << std::endl;
			}
			//TODO - every single "-1"
			if (SR_window[0] == FINAL_SEQUENCE_NUMBER && ack_sequence_number == FINAL_SEQUENCE_NUMBER) {
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