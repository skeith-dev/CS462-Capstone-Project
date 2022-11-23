//
// Created by Spencer Keith on 11/22/22.
//

#ifndef UWEC_CS462_CAPSTONE_PROJECT_PING_H
#define UWEC_CS462_CAPSTONE_PROJECT_PING_H

#include <iostream>
#include <string>
#include <array>


std::string executeSysCommand(const char* command);

float getPingTimeoutInterval(const std::string& result);

#endif //UWEC_CS462_CAPSTONE_PROJECT_PING_H
