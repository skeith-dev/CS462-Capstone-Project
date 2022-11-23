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
 * Writes a section of a file to a char packet[] at the specified location
 * @param packet The packet which the file section is being written to
 * @param filePath The file path of the file whose contents are being written to the packet
 * @param fileSize The size of the file whose contents are being written to the packet
 * @param index Specifies the section of the file to write to the packet
 * @param packetSize The size of the packet
 * @param numOfPackets The number of packets necessary to send the entire file
 */
void writeFileToPacket(char packet[], const std::string& filePath, int fileSize, int index, int packetSize, int numOfPackets);

/**
 * Writes the final packet to be sent by the client socket
 * @param packet The packet which is being written to
 * @param packetSize The size of the packet
 */
void writeFinalPacket(char packet[], int packetSize);

/**
 * Appends the contents of a packet to an output file
 * @param packet The packet whose contents are being appended to the file
 * @param packetSize The size of the packet
 * @param filePath The file path of the file that is being appended to
 */
void appendPacketToFile(const char packet[], int packetSize, const std::string& filePath);

#endif //UWEC_CS462_CAPSTONE_PROJECT_FILEIO_H
