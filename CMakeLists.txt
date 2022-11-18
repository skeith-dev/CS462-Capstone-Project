cmake_minimum_required(VERSION 3.20)
project(UWEC_CS462_Capstone_Project)

set(CMAKE_CXX_STANDARD 14)

add_executable(UWEC_CS462_Capstone_Project_testClient
        src/tests/testClient.cpp
        src/prompts.cpp
        src/prompts.h
        src/fileIO.cpp
        src/fileIO.h
        src/packetIO.cpp
        src/packetIO.h)

add_executable(UWEC_CS452_Capstone_Project_testServer
        src/tests/testServer.cpp
        src/prompts.cpp
        src/prompts.h
        src/fileIO.cpp
        src/fileIO.h
        src/packetIO.cpp
        src/packetIO.h)