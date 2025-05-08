cmake_minimum_required(VERSION 3.10.0)
project(chat_room_build VERSION 0.1.0 LANGUAGES C CXX)

add_executable(srvr src/server.cpp)

include(CTest)
enable_testing()

