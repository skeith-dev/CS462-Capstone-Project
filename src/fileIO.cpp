//
// Created by Spencer Keith on 11/17/22.
//

#include "fileIO.h"


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
    if(seqNum + 1 < fileSizeRangeOfSeqNums) {
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