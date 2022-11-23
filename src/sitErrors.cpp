//
// Created by Spencer Keith on 11/21/22.
//

#include "sitErrors.h"
#include <iostream>
#include <vector>
#include <algorithm>    // std::sort

void generateRandomSitErrors(std::vector<int> &sitErrorsIterations) {

    srand(time(nullptr)); // NOLINT(cert-msc51-cpp)

    //generate a random number between 1 and 3
    int numberOfErrors = (rand() % 3) + 1; // NOLINT(cert-msc30-c, cert-msc50-cpp)

    //insert a random number between 2 and 10 into the vector
    for(int i = 0; i < numberOfErrors; i++) {
        sitErrorsIterations.insert(sitErrorsIterations.end(), (rand() % 10) + 2); // NOLINT(cert-msc30-c, cert-msc50-cpp)
    }

    //remove duplicate elements from the vector
    std::sort(sitErrorsIterations.begin(), sitErrorsIterations.end());
    sitErrorsIterations.erase( std::unique(sitErrorsIterations.begin(), sitErrorsIterations.end() ), sitErrorsIterations.end() );

    std::cout << "Every iteration of each of the following packets will be dropped: ";
    for(int situationalErrorIndex : sitErrorsIterations) {
        std::cout << situationalErrorIndex << " ";
    }
    std::cout << std::endl << std::endl;

}

void generateUserSitErrors(std::vector<int> &sitErrorsIterations) {

    std::string droppedPacketString;
    int droppedPacketCount;

    std::cout << "How many packet iterations do you want to drop:" << std::endl;
    std::getline(std::cin, droppedPacketString);

    droppedPacketCount = std::stoi(droppedPacketString);

    for(int i = 0; i < droppedPacketCount; i++) {
        std::string packetIterationString;
        std::cout << "#" << i + 1 << " packet iteration to be dropped:" << std::endl;
        std::getline(std::cin, packetIterationString);

        sitErrorsIterations.insert(sitErrorsIterations.end(), std::stoi(packetIterationString));
    }

    std::cout << "Every iteration of each of the following packets will be dropped: ";
    for(int situationalErrorIndex : sitErrorsIterations) {
        std::cout << situationalErrorIndex << " ";
    }
    std::cout << std::endl << std::endl;

}

bool checkIfDropPacket(int sitErrorsIterator, const std::vector<int>& sitErrorsIterations) {

    for(int situationalErrorsIteration : sitErrorsIterations) {
        if(sitErrorsIterator > 1 && sitErrorsIterator % situationalErrorsIteration == 0) {
            std::cout << sitErrorsIterator << " % " << situationalErrorsIteration << " = " << 0 << std::endl;
            return true;
        }
    }
    return false;

}