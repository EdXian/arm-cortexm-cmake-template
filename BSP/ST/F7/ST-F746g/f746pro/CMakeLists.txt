
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


project(STM32F74-GDISCO)

enable_language(C ASM)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

file(GLOB CORE_SOURCES
     "Core/Src/*.c"

)

file(GLOB DRIVER_SOURCES
     "Drivers/STM32F7xx_HAL_Driver/Src/*.c"
)


file(GLOB FATFS_SOURCES
     "FATFS/Target/*.c"
     "FATFS/App/*.c"
)


file(GLOB RTT_SOURCES
    RTT/SEGGER/*.c
    RTT/Sample/OS/*.c
    RTT/Config/*.c
    RTT/SEGGER/Syscalls/*.c
    RTT/SEGGER/SEGGER_RTT_ASM_ARMv7M.s
    )

file(GLOB MIDDLE_SOURCES
     "Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Src/*.c"
     "Middlewares/ST/STM32_USB_Host_Library/Core/Src/*.c"
     "Middlewares/Third_Party/FatFs/src/option/*.c"
     "Middlewares/Third_Party/FatFs/src/*.c"
     "Middlewares/Third_Party/FreeRTOS/Source/*.c"
     "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/*.c"
       Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/port.c
	Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/*.c


)

file(GLOB USBHOST_SOURCES
    USB_HOST/App/*.c
    USB_HOST/Target/*.c
)




set(GENERATED_SOURCES_FILES
        ${DRIVER_SOURCES}
        ${FATFS_SOURCES}
        ${MIDDLE_SOURCES}
        ${CORE_SOURCES}
        ${USBHOST_SOURCES}
        ${RTT_SOURCES}
        startup_stm32f746xx.s
        )



set(EXECUTABLE ${PROJECT_NAME}.elf)

add_executable(${EXECUTABLE} ${GENERATED_SOURCES_FILES})
target_compile_definitions(${EXECUTABLE} PRIVATE
    -DUSE_HAL_DRIVER
    -DSTM32F746xx

)


target_include_directories(${EXECUTABLE} PRIVATE

    Core/Inc
    Drivers/CMSIS/Device/ST/STM32F7xx/Include
    Drivers/CMSIS/Include
    Drivers/STM32F7xx_HAL_Driver/Inc
    Drivers/STM32F7xx_HAL_Driver/Inc/Legacy

    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    Middlewares/ST/STM32_USB_Host_Library/Class/CDC/Inc
    Middlewares/ST/STM32_USB_Host_Library/Core/Inc
    Middlewares/Third_Party/FatFs/src
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1
    USB_HOST/App
    USB_HOST/Target

    FATFS/App
    FATFS/Target

    RTT/Config
    RTT/SEGGER
    RTT/Sample/OS
    RTT/SEGGER/Syscalls
    ${CMAKE_SOURCE_DIR}
)




target_compile_options(${EXECUTABLE} PRIVATE

        -mthumb
        -mcpu=cortex-m7
        -mfpu=fpv5-sp-d16
        -mfloat-abi=hard
        -Wall
        -fdata-sections
        -ffunction-sections
        -lc
        -lm
        -lnosys

        -Og
        #-mfpu=fpv4-sp-d16
        #-mfloat-abi=hard
   
)

target_link_options(${EXECUTABLE} PRIVATE
        -mcpu=cortex-m7
        -T${CMAKE_SOURCE_DIR}/STM32F746NGHx_FLASH.ld
        -mfpu=fpv5-sp-d16
        -mfloat-abi=hard
        -g
        -Wl,--gc-sections
        -mthumb
        --specs=nano.specs
        -specs=nosys.specs


        )


add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND arm-none-eabi-size ${EXECUTABLE}
        )



add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND arm-none-eabi-objcopy -O ihex ${EXECUTABLE} ${PROJECT_NAME}.hex
        COMMAND arm-none-eabi-objcopy -O binary ${EXECUTABLE} ${PROJECT_NAME}.bin
        )








