//
// Created by Spencer Keith on 11/17/22.
//

#ifndef UWEC_CS462_CAPSTONE_PROJECT_PROMPTS_H
#define UWEC_CS462_CAPSTONE_PROJECT_PROMPTS_H

#import <iostream>
#import <string>


std::string userStringPrompt(const std::string& prompt);

int userIntPrompt(const std::string& prompt, int min, int max);

float userFloatPrompt(const std::string& prompt, int min, int max);

bool userBoolPrompt(const std::string& prompt);

#endif //UWEC_CS462_CAPSTONE_PROJECT_PROMPTS_H
