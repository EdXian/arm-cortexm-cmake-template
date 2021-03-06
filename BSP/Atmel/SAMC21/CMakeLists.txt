
cmake_minimum_required(VERSION 3.15.0)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

if(MINGW OR CYGWIN OR WIN32)
    set(UTIL_SEARCH_CMD where)
elseif(UNIX OR APPLE)
    set(UTIL_SEARCH_CMD which)
endif()


set(TOOLCHAIN_PREFIX arm-none-eabi-)
set(CMAKE_BUILD_TYPE "Debug")

execute_process(
  COMMAND ${UTIL_SEARCH_CMD} ${TOOLCHAIN_PREFIX}gcc
  OUTPUT_VARIABLE BINUTILS_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

get_filename_component(ARM_TOOLCHAIN_DIR ${BINUTILS_PATH} DIRECTORY)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)


set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)


set(CMAKE_OBJCOPY ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_SIZE_UTIL ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}size CACHE INTERNAL "size tool")


set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


#set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/arm-none-eabi-gcc.cmake CACHE FILEPATH ${CMAKE_SOURCE_DIR}/arm-none-eabi-gcc.cmake )
#message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")


project(SAMC21)

enable_language(C ASM)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB HRI_SOURCES
     hri/*.c
     #   "hri/*.h"
)

file(GLOB HPL_SOURCES
     "hpl/*.c"
    #    "hpl/*.h"
)

file(GLOB HAL_SOURCES
     "hal/*.c"
   # "hal/*.h"

)

file(GLOB MAIN_SOURCES
     "*.c"
        samc21/gcc/system_samc21.c
        samc21/gcc/gcc/startup_samc21.c
 
)
include_directories(
        hri
        hpl
        hal
        CMSIS/Core/Include
        config
        samc21/include/pio
        samc21/include
        samc21/include/instance
        samc21/include/component
        samc21/gcc/gcc
        stdio_redirect
        ${CMAKE_SOURCE_DIR}
)


set(GENERATED_SOURCES_FILES
	${HRI_SOURCES}
	${HPL_SOURCES}
	${HAL_SOURCES}
	${MAIN_SOURCES}
    "atmel_start.h"
        )

#/samc21/gcc/gcc/samc21j18a_flash.ld

set(EXECUTABLE ${PROJECT_NAME}.elf)
add_executable(${EXECUTABLE} ${GENERATED_SOURCES_FILES})


target_compile_definitions(${EXECUTABLE} PRIVATE
        -DDEBUG
        -D__SAMC21J18A__
    )

target_include_directories(${EXECUTABLE} PRIVATE
        hri
        hpl
        hal
        CMSIS/Core/Include
        config
        samc21/include/pio
        samc21/include
        samc21/include/instance
        samc21/include/component
        samc21/gcc/gcc

        stdio_redirect
        ${CMAKE_SOURCE_DIR}
)




target_compile_options(${EXECUTABLE} PRIVATE
        -mcpu=cortex-m0plus #cortex-m4 for same51
        -mthumb
        -mfloat-abi=soft
        -fdata-sections
        -ffunction-sections
        -Wall
        #-mfpu=fpv4-sp-d16
        #-mfloat-abi=hard
        #<$<CONFIG:Debug>:-Og>
)

target_link_options(${EXECUTABLE} PRIVATE
        -T${CMAKE_SOURCE_DIR}/samc21/gcc/gcc/samc21j18a_flash.ld

        -mthumb
        -Wl,-Map=${PROJECT_NAME}.map
        --specs=nano.specs
         #-mfpu=fpv4-sp-d16
        -Wl,--start-group
        -lm
         #-larm_cortexM4lf_math_softfp  ##for cmsis dsp lib
         #-lBio_platform   # bio algorithm
        -Wl,--end-group
        -Wl,--gc-sections
        -mcpu=cortex-m0plus
         -mfloat-abi=soft
        -Wl,--section-start=.text=0x0000


        )

add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND arm-none-eabi-size ${EXECUTABLE})



add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND arm-none-eabi-objcopy -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
        COMMAND arm-none-eabi-objcopy -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin)








