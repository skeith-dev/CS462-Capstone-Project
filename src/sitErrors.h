//
// Created by Spencer Keith on 11/21/22.
//

#ifndef UWEC_CS462_CAPSTONE_PROJECT_SITERRORS_H
#define UWEC_CS462_CAPSTONE_PROJECT_SITERRORS_H

#include <iostream>
#include <vector>


/**
 * Generates and populates a vector with the indices of packets to be dropped
 * @param sitErrorsIterations The vector in which the indices are being stored
 */
void generateRandomSitErrors(std::vector<int> &sitErrorsIterations);

/**
 * Prompts for and populates a vector with the indices of packets to be dropped
 * @param sitErrorsIterations The vector in which the indices are being stored
 */
void generateUserSitErrors(std::vector<int> &sitErrorsIterations);

/**
 * Checks if a packet should be dropped
 * @param sitErrorsIterator The iterator which tracks whether packets should be dropped
 * @param sitErrorsIterations The vector in which the indices of packets to be dropped are stored
 * @return True for yes (DO drop the packet), false for no (DO NOT drop the packet)
 */
bool checkIfDropPacket(int sitErrorsIterator, const std::vector<int>& sitErrorsIterations);

#endif //UWEC_CS462_CAPSTONE_PROJECT_SITERRORS_H
