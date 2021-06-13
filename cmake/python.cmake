
cmake_minimum_required(VERSION 3.16)


find_package (Python3 COMPONENTS Interpreter Development)

if(Python3_FOUND)

message_color("Yellow" "${Python3_EXECUTABLE}")

endif()
