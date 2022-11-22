//
// Created by Spencer Keith on 11/21/22.
//

#ifndef UWEC_CS462_CAPSTONE_PROJECT_SITERRORS_H
#define UWEC_CS462_CAPSTONE_PROJECT_SITERRORS_H

#include <iostream>
#include <vector>


void generateRandomSitErrors(std::vector<int> sitErrorsIterations);

void generateUserSitErrors(std::vector<int> sitErrorsIterations);

bool checkIfDropPacket(int sitErrorsIterator, const std::vector<int>& sitErrorsIterations);

#endif //UWEC_CS462_CAPSTONE_PROJECT_SITERRORS_H
