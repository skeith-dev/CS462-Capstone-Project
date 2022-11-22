//
// Created by Spencer Keith on 11/17/22.
//

#include "fileIO.h"

#define FINAL_SEQUENCE_NUMBER -1


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

void writeFileToPacket(char packet[], const std::string& filePath, int fileSize, int seqNum, int packetSize, int fileSizeRangeOfSeqNums) {

    //create ifstream object
    std::ifstream fileInputStream;
    //open file at filepath in read and binary modes
    fileInputStream.open(filePath, std::ios_base::in | std::ios_base::binary);
    //navigate to section of file beginning at (seqNum * packetSize) offset from beginning
    fileInputStream.seekg(seqNum * packetSize, std::ios_base::beg);

    //create char array for seqNum int
    char seqNumBytes[sizeof(int)];
    std::copy(static_cast<const char*>(static_cast<const void*>(&seqNum)),
              static_cast<const char*>(static_cast<const void*>(&seqNum)) + sizeof(seqNum),
              seqNumBytes);
    //create char array for file contents
    char contentsBytes[packetSize];
    for(int i = 0; i < packetSize; i++) {
        contentsBytes[i] = '\0';
    }
    if(seqNum < fileSizeRangeOfSeqNums) {
        fileInputStream.read(contentsBytes, packetSize);
    } else {
        int remainingBytes = fileSize - (seqNum * packetSize);
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