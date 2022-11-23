//
// Created by Spencer Keith on 11/22/22.
//

#include "ping.h"


std::string executeSysCommand(const char* command) {

    std::array<char, 128> buffer{};
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;

}

float getPingTimeoutInterval(const std::string& result) {

    //find "time=" within result string
    size_t startIndex = result.find(std::string("time="));
    startIndex += std::string("time=").size();
    size_t endIndex = result.find(std::string(" ms"));
    int length = (int) (endIndex - startIndex);

    std::string pingTime = result.substr(startIndex, length);
    std::cout << "Ping time: " << pingTime << " ms" << std::endl;

    return std::stof(pingTime) / 1000;

}