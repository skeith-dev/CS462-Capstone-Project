//
// Created by Spencer Keith on 11/17/22.
//

#ifndef UWEC_CS462_CAPSTONE_PROJECT_FILEIO_H
#define UWEC_CS462_CAPSTONE_PROJECT_FILEIO_H

#include <iostream>
#include <fstream>


/**
 * Opens the file at a specified filePath and returns file size in bytes
 * @param filePath The path of the file being opened
 * @return The size of the file at the specified filePath
 */
int openFile(const std::string& filePath);

/**
 *
 * @param packet
 * @param filePath
 * @param fileSize
 * @param seqNum
 * @param packetSize
 * @param fileSizeRangeOfSeqNums
 */
void writeFileToPacket(char packet[], const std::string& filePath, int fileSize, int seqNum, int packetSize, int fileSizeRangeOfSeqNums);

#endif //UWEC_CS462_CAPSTONE_PROJECT_FILEIO_H
