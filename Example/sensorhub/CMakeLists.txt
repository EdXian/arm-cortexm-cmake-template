cmake_minimum_required(VERSION 3.16.0)

set(PROJECT_VERSION 1.0.0 CACHE STRING "set the version number for projects")
project(samd21 VERSION ${PROJECT_VERSION}  LANGUAGES C ASM )


include(utils)
include(bsp)
include(jlink)
include(uf2)

set(CMAKE_C_STANDARD 99)
enable_language(ASM)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

set(USE_TINYUSB ON CACHE BOOL "True if use tinyusb")


message_color(Green "ARM_C_COMPILER : ${CMAKE_C_COMPILER}")
message_color(Green "ARM_CXX_COMPILER : ${CMAKE_CXX_COMPILER}")
message_color(Green "ARM_ASM_COMPILER : ${CMAKE_ASM_COMPILER}")

set(CMSIS_PATH ${THIRDPARTY_ROOT_PATH}/CMSIS_5/CMSIS/Core/Include)

set(DEPEN_DIR
    ${CMSIS_PATH}/

    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/include
    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/portable/ARM_CM0

    ${CMAKE_CURRENT_SOURCE_DIR}/hw
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/include
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/include/pio
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/include/instance
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/include/component
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hri
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hpl
${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hpl/gclk
${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hpl/pm
${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hpl/sysctrl
${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hal/include
#    hw/mcu/microchip/samd21/hal/src/hal_atomic.c
${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hal/utils/include



    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/class
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/device
    ${CMAKE_CURRENT_SOURCE_DIR}/src/host
    ${CMAKE_CURRENT_SOURCE_DIR}/src/osal


    ${CMAKE_CURRENT_SOURCE_DIR}/hw/bsp/samd21/boards/atsamd21_xpro


    ${CMAKE_CURRENT_SOURCE_DIR}/examples/device/cdc_msc_freertos/src




   ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/
   ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/config
   ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/include
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hal/utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hpl/pm/
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hpl/port
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/mcu/microchip/samd21/hri
    )

set(CPU_FLAGS -mthumb -mcpu=cortex-m0plus  -g -mfloat-abi=soft -mfpu=fpv4-sp-d16) #-mfloat-abi=soft -mfpu=fpv4-sp-d16
set(DEBUG_FLAGS 3)
set(OPTIMIZATION_FLAGS O0)


set(CFLAGS
#    -nostdlib
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

    -flto
    -mthumb
    -mabi=aapcs
    -mcpu=cortex-m0plus
    #-nostdlib
    -nostartfiles
    -DCONF_DFLL_OVERWRITE_CALIBRATION=0
    -DCFG_TUSB_MCU=OPT_MCU_SAMD21

    --specs=nosys.specs
     --specs=nano.specs



    )


set(LDFLAGS
     -T ${CMAKE_CURRENT_SOURCE_DIR}/hw/bsp/samd21/boards/atsamd21_xpro/samd21j18a_flash.ld
    -g${DEBUG_FLAGS}
    ${CPU_FLAGS}
    #-nostdlib

    -u_printf_float
    --specs=nosys.specs
     --specs=nano.specs
    -Wl,--gc-sections
)

file(GLOB   FREERTOS_SOURCE

    ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/*.c
     ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/portable/ARM_CM0/port.c
     ${THIRDPARTY_ROOT_PATH}/FreeRTOS/Source/portable/MemMang/heap_4.c

    )

set(SOURCE_FILE
   ${FREERTOS_SOURCE}

   src/portable/microchip/samd/dcd_samd.c
   hw/mcu/microchip/samd21/gcc/gcc/startup_samd21.c
   hw/mcu/microchip/samd21/gcc/system_samd21.c
   hw/mcu/microchip/samd21/hpl/gclk/hpl_gclk.c
   hw/mcu/microchip/samd21/hpl/pm/hpl_pm.c
   hw/mcu/microchip/samd21/hpl/sysctrl/hpl_sysctrl.c
   hw/mcu/microchip/samd21/hal/src/hal_atomic.c
hw/mcu/microchip/samd21/hal/utils/src/utils_assert.c
#hw/mcu/microchip/samd21/hal/utils/src/utils_syscalls.c
   src/tusb.c
           src/common/tusb_fifo.c
           src/device/usbd.c
           src/device/usbd_control.c
           src/class/audio/audio_device.c
           src/class/cdc/cdc_device.c
           src/class/dfu/dfu_device.c
           src/class/dfu/dfu_rt_device.c
           src/class/hid/hid_device.c
           src/class/midi/midi_device.c
           src/class/msc/msc_device.c
           src/class/net/net_device.c
           src/class/usbtmc/usbtmc_device.c
           src/class/vendor/vendor_device.c


    ${CMAKE_CURRENT_SOURCE_DIR}/hw/bsp/board.c
    ${CMAKE_CURRENT_SOURCE_DIR}/hw/bsp/samd21/family.c
#${CMAKE_CURRENT_SOURCE_DIR}/src/portable/microchip/samd/dcd_samd.c

    )

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/device/cdc_msc_freertos/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/device/cdc_msc_freertos/src/msc_disk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/device/cdc_msc_freertos/src/usb_descriptors.c
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/device/cdc_msc_freertos/src/freertos_hook.c
    ${SOURCE_FILE}
    )



target_include_directories(
    ${PROJECT_NAME} PUBLIC
    ${DEPEN_DIR}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        -DDEBUG
        -DCFG_TUSB_MCU=OPT_MCU_SAMD21
        -D__SAMD21J18A__
        -DUSE_SIMPLE_ASSERT
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
    m
)

#set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/main")
generate_jlink_basic_cmd("ATSAMD21G18A")
generate_output_file("${PROJECT_NAME}")
generate_jlink_cmd(${PROJECT_NAME}  "ATSAMD21G18A" "${PROJECT_NAME}.bin" "0x00000000")
install_output_file("${PROJECT_NAME}" ${CMAKE_PROJECT_VERSION})
generate_uf2_file(${PROJECT_NAME} "0x00000000")

#add_subdirectory(example)
