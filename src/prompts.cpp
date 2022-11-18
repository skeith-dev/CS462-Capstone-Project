//
// Created by Spencer Keith on 11/17/22.
//

#include "prompts.h"


std::string userStringPrompt(const std::string& prompt) {

    std::cout << prompt << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    return responseString;

}

int userIntPrompt(const std::string& prompt, int min, int max) {

    std::cout << prompt << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    int responseInt = std::stoi(responseString);
    while (responseInt < min || responseInt > max) {
        std::cout << "Please enter a value between " << 0 << " and " << 1 << std::endl;
        responseString.clear();
        std::getline(std::cin, responseString);
        responseInt = std::stoi(responseString);
    }

    return responseInt;

}

bool userBoolPrompt(const std::string& prompt) {

    std::cout << prompt << std::endl;

    std::string responseString;
    std::getline(std::cin, responseString);

    int responseInt = std::stoi(responseString);
    while(responseInt < 0 || responseInt > 1) {
        std::cout << "Please enter a value between " << 0 << " and " << 1 << std::endl;
        responseString.clear();
        std::getline(std::cin, responseString);
        responseInt = std::stoi(responseString);
    }
    return std::stoi(responseString) == 1;

}