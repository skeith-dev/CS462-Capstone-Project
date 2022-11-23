//
// Created by Spencer Keith on 11/17/22.
//

#include "fileIO.h"

#define FINAL_SEQUENCE_NUMBER (-1)

int openFile(const std::string& filePath) {

    std::ifstream fileInputStream;

    fileInputStream.open(filePath, std::ios_base::in | std::ios_base::binary);
    if (fileInputStream.fail()) {
        throw std::fstream::failure("Failed while opening file " + filePath);
    }

    fileInputStream.seekg(0, fileInputStream.end);
    int fileSize = (int) fileInputStream.tellg();
    fileInputStream.close();

    std::cout << std::endl << "File to deliver: " << filePath << std::endl << "File size: " << fileSize << " bytes" << std::endl << std::endl;

    return fileSize;

}

//NOTE: num_cycles became number of cycles * window size for simplicity.
void writeFileToPacket(char packet[], const std::string& filePath, int fileSize, int seqNum, int num_cycles, int iterator, int packetSize, int fileSizeRangeOfSeqNums) {

    //create ifstream object
    std::ifstream fileInputStream;
    //open file at filepath in read and binary modes
    fileInputStream.open(filePath, std::ios_base::in | std::ios_base::binary);
    //navigate to section of file beginning at (seqNum * packetSize) offset from beginning
	int start = (seqNum + num_cycles) * packetSize;
    fileInputStream.seekg(start, std::ios_base::beg);
	
	//std::cout << "The current number to read: " << start << std::endl;
	//std::cout << "The number of cycles: " << num_cycles << std::endl << std::endl << std::endl;

    //create char array for seqNum int
    char seqNumBytes[sizeof(int)];
	if (seqNum == -1) {
		seqNumBytes[0] = 0;
		seqNumBytes[1] = 1;
		seqNumBytes[2] = 1;
		seqNumBytes[3] = 0;
	} else {
		for (int i=sizeof(int)-1; i >= 0; i--) {
			if(seqNum - 127 > 0) {
				seqNumBytes[i] = 127;
				seqNum = seqNum - 127;
			} else if (seqNum - 127 < 0){
				seqNumBytes[i] = seqNum;
				seqNum = 0;
			} else {
				seqNumBytes[i] = 0;
			}
		}
	}
    //create char array for valid bool
    char validBytes[sizeof(bool)];
	validBytes[0] = '1';
    //create char array for file contents
    char contentsBytes[packetSize];

    if(seqNum + 1 < fileSizeRangeOfSeqNums) {
        fileInputStream.read(contentsBytes, packetSize);
        int remainingBytes = fileSize - (seqNum * packetSize);
		if (remainingBytes < packetSize) {
			contentsBytes[remainingBytes] = 4;
		}
    } else {
        int remainingBytes = fileSize - (seqNum * packetSize);
        //std::cout << "REMAINING BYTES: " << remainingBytes << std::endl;
        fileInputStream.read(contentsBytes, remainingBytes);
		if(remainingBytes>0) {
			contentsBytes[remainingBytes] = 4;
		} else {
			contentsBytes[0] = 4;
		}
		
		//TODO
		std::cout << "Bytes became: " << std::endl;
		for (int i=0; i<packetSize; i++) {
			std::cout << contentsBytes[i] << ", int=" << int(contentsBytes[i]) << std::endl;
		}
		exit(0);
		
    }
    //construct char array "packet"
    for(int i = 0; i < sizeof(int); i++) {
        packet[i] = seqNumBytes[i];
		//std::cout << "seqNumBytes[" << i << "]: " << seqNumBytes[i] << std::endl;
    }
    for(int i = 0; i < sizeof(bool); i++) {
        packet[i + sizeof(int)] = validBytes[i];
		//std::cout << "validBytes[" << i << "]: " << validBytes[i] << std::endl;
    }
    for(int i = 0; i < packetSize; i++) {
        packet[i + sizeof(int) + sizeof(bool)] = contentsBytes[i];
    }
	
    fileInputStream.close();

}

//TODO - implement char = 4; (EOT) for the input to this function
void writePacketToFile(bool append, const std::string& message, const std::string& filePath) {

    std::ofstream fileOutputStream;
    if(append) {
        fileOutputStream.open(filePath, std::ios_base::app);
    } else {
        fileOutputStream.open(filePath);
    }
	
	//with breaks
	int message_end = 0;
	for (int i=0; i<message.length(); i++) {
		if (message[i] == char(4)) {
			break;
		}
		message_end++;
	}

    fileOutputStream << message.substr(0, message_end);

    fileOutputStream.close();

}

// // //
void writeFileToPacketSW(char packet[], const std::string& filePath, int fileSize, int iterator, int packetSize, int numOfPackets) {

    //create ifstream object
    std::ifstream fileInputStream;
    //open file at filepath in read and binary modes
    fileInputStream.open(filePath, std::ios_base::in | std::ios_base::binary);
    //navigate to section of file beginning at (iterator * packetSize) offset from beginning
    fileInputStream.seekg(iterator * packetSize, std::ios_base::beg);

    //create char array for seqNum int
    char seqNumBytes[sizeof(int)];
    std::copy(static_cast<const char*>(static_cast<const void*>(&iterator)),
              static_cast<const char*>(static_cast<const void*>(&iterator)) + sizeof(iterator),
              seqNumBytes);
    //create char array for file contents
    char contentsBytes[packetSize];
    for(int i = 0; i < packetSize; i++) {
        contentsBytes[i] = '\0';
    }
    if(iterator < numOfPackets) {
        fileInputStream.read(contentsBytes, packetSize);
    } else {
        int remainingBytes = fileSize - (iterator * packetSize);
        std::cout << "REMAINING BYTES: " << remainingBytes << std::endl;
        fileInputStream.read(contentsBytes, remainingBytes);
    }
    //construct char array "packet"
    for(int i = 0; i < sizeof(int); i++) {
        packet[i] = seqNumBytes[i];
    }
    for(int i = 0; i < packetSize; i++) {
        packet[i + sizeof(int)] = contentsBytes[i];
    }

    fileInputStream.close();

}

void writeFinalPacket(char packet[], int packetSize) {

    //create char array for FINAL_SEQUENCE_NUMBER
    int seqNum = FINAL_SEQUENCE_NUMBER;
    char seqNumBytes[sizeof(int)];
    std::copy(static_cast<const char*>(static_cast<const void*>(&seqNum)),
              static_cast<const char*>(static_cast<const void*>(&seqNum)) + sizeof(seqNum),
              seqNumBytes);
    //construct char array "packet"
    for(int i = 0; i < sizeof(int); i++) {
        packet[i] = seqNumBytes[i];
    }
    for(int i = 0; i < packetSize; i++) {
        packet[i + sizeof(int)] = '\0';
    }

}

void appendPacketToFile(const char packet[], int packetSize, const std::string& filePath) {

    std::ofstream fileOutputStream;
    fileOutputStream.open(filePath, std::ios_base::app);

    char contents[packetSize];
    for(int i = 0; i < packetSize; i++) {
        if(packet[sizeof(int) + i] != '\0') {
            contents[i] = packet[sizeof(int) + i];
        } else {
            contents[i] = '\0';
        }
    }
    //without the line below, extra characters are printed because the fileOutputStream
    //does not know where to terminate
    contents[packetSize] = '\0';
    std::string message(contents);

    fileOutputStream << message;

    fileOutputStream.close();

}




/* INFO */

//Each packet contains:
//[int=seqNum, 4 bytes], [bool=valid, 1 byte], [char*=contents, packetSize bytes]