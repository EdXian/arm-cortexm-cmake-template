cmake_minimum_required(VERSION 3.16.0)

set(PROJECT_VERSION 1.0.0 CACHE STRING "set the version number for projects")
project(samd21 VERSION ${PROJECT_VERSION}  LANGUAGES C ASM )


include(utils)
include(bsp)
include(jlink)


set(CMAKE_C_STANDARD 99)
enable_language(ASM)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(USE_TINYUSB ON CACHE BOOL "True if use tinyusb")


message_color(Green "ARM_C_COMPILER : ${CMAKE_C_COMPILER}")
message_color(Green "ARM_CXX_COMPILER : ${CMAKE_CXX_COMPILER}")
message_color(Green "ARM_ASM_COMPILER : ${CMAKE_ASM_COMPILER}")


set(BSP_PATH  ${BSP_ROOT_PATH}/Atmel/SAMD21)

set(CMSIS_PATH ${THIRDPARTY_ROOT_PATH}/CMSIS_5/CMSIS/Core/Include)

set(STARTUP_FILE ${BSP_PATH}/samd21a/gcc/gcc/startup_samd21.c)
set(SYSTEM_FILE ${BSP_PATH}/samd21a/gcc/system_samd21.c  )

message_color(Red ${PROJECT_NAME})
if(USE_TINYUSB)
    set(TINYUSB_PATH  ${THIRDPARTY_ROOT_PATH}/tinyusb)
    file(GLOB TINYUSB_CDC_SOURCES ${TINYUSB_PATH}/src/class/cdc/*.c)
    file(GLOB TINYUSB_MSC_SOURCES ${TINYUSB_PATH}/src/class/msc/*.c)
    file(GLOB TINYUSB_COMMON_SOURCES ${TINYUSB_PATH}/src/common/*.c)
    file(GLOB TINYUSB_DEVICE_SOURCES ${TINYUSB_PATH}/src/device/*.c)
    set(TINYUSB_PORTABLE_SOURCES ${TINYUSB_PATH}/src/portable/microchip/samd/dcd_samd.c)

    set(TINYUSB_INCLUDE_PATH
        ${TINYUSB_PATH}/src
        ${TINYUSB_PATH}/hw/bsp
        )

    set(TINYUSB_SOURCES
        ${TINYUSB_CDC_SOURCES}
        ${TINYUSB_MSC_SOURCES}
        ${TINYUSB_COMMON_SOURCES}
        ${TINYUSB_DEVICE_SOURCES}
        ${TINYUSB_PORTABLE_SOURCES}
        ${TINYUSB_PATH}/src/tusb.c
        ${TINYUSB_PATH}/hw/bsp/board.c
        )

endif(USE_TINYUSB)




file(GLOB RTT_SOURCES
    ${THIRDPARTY_ROOT_PATH}/RTT/SEGGER/SEGGER_RTT.c
    ${THIRDPARTY_ROOT_PATH}/RTT/SEGGER/SEGGER_RTT_printf.c
    )


file(GLOB FREERTOS_SOURCES


    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/*.c

    )

set(DRIVE_SOURCE_FILE

    )





file( GLOB
    HAL_SOURCES

    ${BSP_PATH}/hal/src/*.c
    ${BSP_PATH}/hal/utils/src/*.c

    ${BSP_PATH}/hpl/core/*.c
    ${BSP_PATH}/hpl/dmac/*.c
    ${BSP_PATH}/hpl/gclk/*.c
    ${BSP_PATH}/hpl/pm/*.c
    ${BSP_PATH}/hpl/port/*.c
    ${BSP_PATH}/hpl/sysctrl/*.c
    ${BSP_PATH}/hpl/usb/*.c

)

file( GLOB
    USB_SOURCES
    ${BSP_PATH}/usb/*.c
    ${BSP_PATH}/usb/class/cdc/device/*.c
    ${BSP_PATH}/usb/device/*.c
    )

set(
    USB_INCLUDE
    ${BSP_PATH}/usb
    ${BSP_PATH}/usb/device
    ${BSP_PATH}/usb/class/cdc/device
    ${BSP_PATH}/usb/class/cdc

    )


set(DRIVER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/spi.c
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/uart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/driver/clock.c
    )


set(SOURCE_FILES
    ${STARTUP_FILE}
    ${HAL_SOURCES}
    ${SYSTEM_FILE}
    ${USB_SOURCES}
    ${DRIVER_FILES}
    #${RTT_SOURCES}
#    ${FREERTOS_SOURCES}
    ${TINYUSB_SOURCES}

#    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/portable/ARM_CM0/port.c
#    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/portable/MemMang/heap_4.c
    )



set(
    HAL_INCLUDE

    ${BSP_PATH}/hal/include
    ${BSP_PATH}/hal/utils/include

    ${BSP_PATH}/hpl/core
    ${BSP_PATH}/hpl/gclk
    ${BSP_PATH}/hpl/pm
    ${BSP_PATH}/hpl/port
    ${BSP_PATH}/hpl/sysctrl
    ${BSP_PATH}/hpl/usb

    ${BSP_PATH}/hri

    ${USB_INCLUDE}

    ${BSP_PATH}/samd21a/include
    ${BSP_PATH}/samd21a/include/pio
    ${BSP_PATH}/samd21a/include/component
    ${BSP_PATH}/samd21a/include/instance

)

set(MAVLINK_INCLUDE

    #${CMAKE_SOURCE_DIR}/serial/minimal

    )



set(DEPEN_DIR


    ${CMAKE_CURRENT_SOURCE_DIR}/config
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_SOURCE_DIR}/driver
    ${HAL_INCLUDE}
#    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/portable/ARM_CM0
#    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source
#    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/include
    ${TINYUSB_INCLUDE_PATH}
#    ${THIRDPARTY_ROOT_PATH}/RTT/Config
#    ${THIRDPARTY_ROOT_PATH}/RTT/SEGGER
    ${MAVLINK_INCLUDE}
    ${CMSIS_PATH}/




    )

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
#-specs=nano.specs
#-specs=nosys.specs
##     -flto

##    -mabi=aapcs
#      -nostartfiles
#     -DCONF_DFLL_OVERWRITE_CALIBRATION=0
#    -DCFG_TUSB_MCU=OPT_MCU_SAMD21




    )


set(LDFLAGS
     -T${BSP_PATH}/samd21a/gcc/gcc/samd21g18a_flash.ld
    -g${DEBUG_FLAGS}
    ${CPU_FLAGS}
    --specs=nano.specs
    -Wl,--gc-sections
)



add_executable(${PROJECT_NAME}
    usb_cdc_echo_main.c
#    freertos_hook.c
    msc_disk.c
    usb_descriptors.c
    family.c
#    atmel_start.c
#    usb_start.c
#    driver_init.c
    ${SOURCE_FILES}
    )



target_include_directories(
    ${PROJECT_NAME} PUBLIC
    ${DEPEN_DIR}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        -DDEBUG
        -DCFG_TUSB_MCU=OPT_MCU_SAMD21
        -D__SAMD21G18A__
    )


# set CMAKE_C_FLAGS




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

#set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/main")
generate_jlink_basic_cmd("ATSAMD21G18A")
generate_output_file("${PROJECT_NAME}")
generate_jlink_cmd(${PROJECT_NAME}  "ATSAMD21G18A" "${PROJECT_NAME}.bin" "0x00000000")
install_output_file("${PROJECT_NAME}" ${CMAKE_PROJECT_VERSION})
#generate_uf2_file(${PROJECT_NAME} "0x00000000")

#add_subdirectory(example)
