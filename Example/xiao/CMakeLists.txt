cmake_minimum_required(VERSION 3.16.0)

set(PROJECT_VERSION 1.0.0 CACHE STRING "set the version number for projects")
project(xiao VERSION ${PROJECT_VERSION}  LANGUAGES C ASM )


include(utils)
include(bsp)
include(jlink)
include(doxygen)

set(CMAKE_C_STANDARD 99)
enable_language(ASM)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")


message_color(Green "ARM_C_COMPILER : ${CMAKE_C_COMPILER}")
message_color(Green "ARM_CXX_COMPILER : ${CMAKE_CXX_COMPILER}")
message_color(Green "ARM_ASM_COMPILER : ${CMAKE_ASM_COMPILER}")


set(BSP_PATH  ${BSP_ROOT_PATH}/Atmel/SAMD21/samd21a)


set(CONFIG_PATH ${BSP_PATH}/Config)

set(TARGET_COMPONENTS_PATH   ${BSP_PATH}/include )

set(CMSIS_PATH ${THIRDPARTY_ROOT_PATH}/CMSIS_5/CMSIS/Core/Include)

set(STARTUP_FILE ${BSP_PATH}/gcc/gcc/startup_samd21.c)
set(SYSTEM_FILE ${BSP_PATH}/gcc/system_samd21.c  )

message_color(Red ${PROJECT_NAME})

file(GLOB RTT_SOURCES
    ${THIRDPARTY_ROOT_PATH}/RTT/SEGGER/SEGGER_RTT.c
    ${THIRDPARTY_ROOT_PATH}/RTT/SEGGER/SEGGER_RTT_printf.c
    )


file(GLOB FREERTOS_SOURCES

    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/portable/ARM_CM0/port.c
    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/*.c
    )


set(SOURCE_FILES
    ${STARTUP_FILE}
    ${SYSTEM_FILE}
    ${RTT_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/uart.c
    main.c
    )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
message_color(Red ${THIRDPARTY_ROOT_PATH})
target_include_directories(
    ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/driver
    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/portable/ARM_CM0
    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source
    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/include

    ${THIRDPARTY_ROOT_PATH}/RTT/Config
    ${THIRDPARTY_ROOT_PATH}/RTT/SEGGER

    ${CMSIS_PATH}/
    ${BSP_PATH}/include
    ${BSP_PATH}/include/pio
    ${BSP_PATH}/include/component
    ${BSP_PATH}/include/instance
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        -DDEBUG
        -D__SAMD21G18A__
    )


# set CMAKE_C_FLAGS

set(CPU_FLAGS -mthumb -mcpu=cortex-m0plus  -g ) #-mfloat-abi=soft -mfpu=fpv4-sp-d16
set(DEBUG_FLAGS 3)
set(OPTIMIZATION_FLAGS O0)


set(CFLAGS
    -nostdlib
    -x c
    -c
    -std=gnu99
    ${CPU_FLAGS}
    -Wall
    -ffunction-sections
    -fdata-sections
    -g${DEBUG_FLAGS}
    -${OPTIMIZATION_FLAGS}
    -mlong-calls

    )


set(LDFLAGS
    -T${BSP_PATH}/gcc/gcc/samd21g18a_flash.ld
    -g${DEBUG_FLAGS}
    ${CPU_FLAGS}
    --specs=nano.specs
    -Wl,--gc-sections
)


target_compile_options(${PROJECT_NAME} PUBLIC
    ${CFLAGS}
)

#CMAKE_C_LINK_FLAGS

target_link_options(${PROJECT_NAME} PUBLIC
         ${LDFLAGS}
)

target_link_libraries(
    ${PROJECT_NAME} PUBLIC
)

generate_jlink_basic_cmd("ATSAMD21G18A")
generate_output_file("${PROJECT_NAME}")
generate_jlink_cmd(${PROJECT_NAME}  "ATSAMD21G18A" "${PROJECT_NAME}.bin" "0x00002000")
install_output_file("${PROJECT_NAME}" ${CMAKE_PROJECT_VERSION})
#generate_uf2_file(${PROJECT_NAME} "0x00002000")
