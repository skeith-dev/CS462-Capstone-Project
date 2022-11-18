//
// Created by Spencer Keith on 11/17/22.
//

#ifndef UWEC_CS462_CAPSTONE_PROJECT_PROMPTS_H
#define UWEC_CS462_CAPSTONE_PROJECT_PROMPTS_H

#import <iostream>
#import <string>


std::string ipAddressPrompt();

int portNumPrompt();

int protocolTypePrompt();

int packetSizePrompt();

int timeoutIntervalPrompt();

int slidingWindowSizePrompt();

int rangeOfSequenceNumbersPrompt(int fileSizeRangeOfSequenceNumbers);

int situationalErrorsPrompt();

std::string inputFilePathPrompt();

std::string outputFilePathPrompt();

bool quitPrompt();

#endif //UWEC_CS462_CAPSTONE_PROJECT_PROMPTS_H
