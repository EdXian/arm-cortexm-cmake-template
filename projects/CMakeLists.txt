cmake_minimum_required(VERSION 3.16)
project(STEVAL VERSION  1.0.0 LANGUAGES C ASM)

include(utils)
include(jlink)
include(bsp)

set(CMAKE_C_STANDARD 99)
enable_language(ASM)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

generate_jlink_basic_cmd("STM32L476RG")

#set(CMAKE_PREFIX_PATH   "/opt/gcc-arm-none-eabi/arm-none-eabi")
#set(CMAKE_INCLUDE_PATH "/opt/gcc-arm-none-eabi/arm-none-eabi/include")

#message_color(Red ${CMAKE_PREFIX_PATH})
#message_color(Red ${CMAKE_INCLUDE_PATH})


#== generate the test executable file ==#
add_subdirectory(${CMAKE_SOURCE_DIR}/DataLog)
