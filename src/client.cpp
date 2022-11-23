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
int packet_size, timeout_interval, window_size, sequence_range, port_num, fileSize, fileSizeRangeOfSeqNums, slidingWindowSize;
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
        int portNum = 9091;//userIntPrompt("What is the port number of the target server:", 0, 9999);
        serverAddress.sin_port = htons(portNum);

        //0 for S&W, 1 for SR
        int protocolType = 1;//userIntPrompt("Type of protocol, S&W (0) or SR (1):", 0, 1);


        //specified size of packets to be sent
		//relates only to the contents, not the header. More bytes will be sent than this
        packet_size = 30;//userIntPrompt("Size of packets:", 1, INT_MAX);

        //user-specified (0+) or default (-1)
        timeout_interval = 9999;//userIntPrompt("Timeout interval, user-specified or ping calculated (-1) (measured in ms):", -1, INT_MAX);
        //ex. [1, 2, 3, 4, 5, 6, 7, 8], size = 8

		//TODO - he says not infinite, but the max could really be anything
		//TODO - if sequenceNum calculations change, this can be higher than 1015(ish), but 1000 isn't bad
		sequence_range = 80;//userIntPrompt("Sequence range: ", 2, 1000);

        if(protocolType > 0) {
            window_size = 20;//userIntPrompt("Size of sliding window:", 1, sequence_range/2);
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
        fileSizeRangeOfSeqNums = fileSize / packet_size;//+ fileSize % packet_size;
		if (fileSizeRangeOfSeqNums < 1) {
			fileSizeRangeOfSeqNums = 1;
		} else if ((fileSize % packet_size) > 0) fileSizeRangeOfSeqNums++;

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
		std::cout << std::endl;
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
    int cycles_completed = 1;
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

	std::cout << std::endl << "Num packets needed to send file:" << fileSizeRangeOfSeqNums << std::endl;
	
	int SR_window[window_size];
	int status[window_size] = { 0 }; //0==unsent,1==sent,2==received,-1==END
	std::chrono::_V2::system_clock::time_point sent_times[window_size];
	
	//populate SR_window with sequence numbers
	for (int i = 0; i < window_size; i++) {
		if (i >= fileSizeRangeOfSeqNums) {
			status[i] = FINAL_SEQUENCE_NUMBER;
		} else {
			status[i] = 0;
		}
		SR_window[i] = i;
	}
	
	double sent_data = 0;
	double num_retransmitted = 0;
	
	std::cout << std::endl << std::endl;

	//Don't take poseidon down, just keep this failsafe implemented in case something fails but it keeps running
	int FAILSAFE = 0;
    while(FAILSAFE < 1000000000) {
		FAILSAFE++;

		//send the packet
		if (status[window_position] == 0 && status[window_position] != FINAL_SEQUENCE_NUMBER){
			//the packet hasn't been sent yet, send it
			int packetArrSize = (int) (packet_size + sizeof(int) + sizeof(bool)); //TODO -> + sizeof(CHECKSUM)
			char packet[packetArrSize];
			int offset = sequence_range * cycles_completed - sequence_range;
			writeFileToPacket(packet, filePath, fileSize, SR_window[window_position], offset, iterator, packet_size, fileSizeRangeOfSeqNums);
			sent_data += sendPacket(clientSocket, packet, SR_window[window_position], packet_size);
			
			status[window_position] = 1;
			sent_times[window_position] = std::chrono::system_clock::now();
			
		} else if (status[window_position] == 1 && (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - sent_times[window_position])).count() > timeout_interval) {
			//the packet was sent and timed out, send again
			std::cout << "** Packet " << SR_window[window_position] << " timed out. Sending again **" << std::endl;
			num_retransmitted += 1;

			int packetArrSize = (int) (packet_size + sizeof(int) + sizeof(bool)); //TODO -> + sizeof(CHECKSUM)
			char packet[packetArrSize];
			int offset = sequence_range * cycles_completed - sequence_range;
			writeFileToPacket(packet, filePath, fileSize, SR_window[window_position], offset, iterator, packet_size, fileSizeRangeOfSeqNums);
			sent_data += sendPacket(clientSocket, packet, SR_window[window_position], packet_size);

			sent_times[window_position] = std::chrono::system_clock::now();

		}
		//the packet is currently being sent, or it was already accepted. Either way, check the next.
		window_position = (window_position+1) % window_size;

		int packetArrSize = (int) (1 + sizeof(int) + sizeof(bool)); //TODO -> + sizeof(CHECKSUM)? Not for getting acks...
		char ack[packetArrSize];

		//check for acks
        if(recv(clientSocket, &ack, sizeof(ack), MSG_DONTWAIT) > 0) {
			//TODO - checksum changes here too
			int ack_sequence_number = 0;
			bool ack_valid = int(ack[sizeof(int)]);

			//this could be a function (probably unecessary, but it comes up in different files)
			//get the sequence number of the ack
			if (ack[0]==0 && ack[1]==1 && ack[2]==1 && ack[3]==0) {//final sequence num reached
				ack_sequence_number = -1;
			} else {//TODO - this is a poor solution for the end of sequence, max 508
				for (int i=0; i < sizeof(int); i++) {
					ack_sequence_number += ack[i];
				}
			}

			if (ack_sequence_number == FINAL_SEQUENCE_NUMBER) {
				continue;
			}
			//if the ack is outside the window, something is wrong... Skip this iteration
			if(ack_sequence_number < SR_window[0] && ack_sequence_number > SR_window[window_size-1]) {
				continue;
			}
            std::cout << "Received ack #" << ack_sequence_number << std::endl;
            iterator++;
			//OPTIMIZE
			for(int index = 0; index < window_size; index++) {
				if(ack_sequence_number == SR_window[index]) {
					status[index] = 2;
					break;
				}
			}
	
			//check for window (does it need to move?)
			//below logic => if received at the window's start is -1, then move window up until it isn't
			if (status[0] == 2) {
				while (status[0] == 2) {
				//OPTIMIZE - while and for loop could be separated
					for(int i=0; i<window_size-1; i++){
						SR_window[i] = SR_window[i+1];
						sent_times[i] = sent_times[i+1];
						status[i] = status[i+1];

					}
					
					if (status[window_size-1] == FINAL_SEQUENCE_NUMBER) {						
						status[window_size-1] = FINAL_SEQUENCE_NUMBER;
					} else {
						status[window_size-1] = 0;
					}
					SR_window[window_size-1] = SR_window[window_size-1]+1;

					if (SR_window[window_size-1] >= sequence_range) {
						SR_window[window_size-1] = 0;
						cycles_completed++;
					}

					window_position = window_position-1;
					if (window_position < 0) window_position = window_size-1;
				}
				// Print the (new) current window
				std::cout << "Current window: [";
				for (int i = 0; i < window_size-1; i++) 
					std::cout << SR_window[i] << ", ";
				std::cout << SR_window[window_size-1] << "]" << std::endl;
			}
			
			//if all packets sent, break
			if (iterator == fileSizeRangeOfSeqNums) {
				//std::cout << "done" << std::endl;

				int packetArrSize = (int) (packet_size + sizeof(int) + sizeof(bool)); //TODO -> + sizeof(CHECKSUM)
				char packet[packetArrSize];
				writeFileToPacket(packet, filePath, fileSize, FINAL_SEQUENCE_NUMBER, (window_size*(cycles_completed-1)), iterator, packet_size, fileSizeRangeOfSeqNums);
				sendPacket(clientSocket, packet, FINAL_SEQUENCE_NUMBER, packet_size);

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
	double total_time = elapsedSeconds.count();//std::chrono::duration_cast<seconds>
    std::cout << std::endl << "Number of original packets sent: " << iterator-num_retransmitted << std::endl;
    std::cout << "Number of retransmitted packets: " << num_retransmitted << std::endl;
    std::cout << "Total execution time (seconds): " << total_time << std::endl;
    std::cout << "Total throughput (b/s): " << sent_data/total_time << std::endl;
    std::cout << "Total throughput (b/s): " << sent_data/total_time << std::endl;

    close(client_fd);

}

