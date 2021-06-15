cmake_minimum_required(VERSION 3.16.0)


project(sensortile VERSION  1.0.0 LANGUAGES C ASM)

include(${CMAKE_MODULE_PATH}/utils.cmake)
include(${CMAKE_MODULE_PATH}/bsp.cmake)
include(${CMAKE_MODULE_PATH}/jlink.cmake)
include(${CMAKE_MODULE_PATH}/doxygen.cmake)



message_color(Green "ARM_C_COMPILER : ${CMAKE_C_COMPILER}")
message_color(Green "ARM_CXX_COMPILER : ${CMAKE_CXX_COMPILER}")
message_color(Green "ARM_ASM_COMPILER : ${CMAKE_ASM_COMPILER}")



set( BSP_PATH  ${BSP_ROOT_PATH}/ST)

set(CMSIS_PATH ${THIRDPARTY_ROOT_PATH}/CMSIS_5/CMSIS/Core/Include)



file(GLOB HAL_SOURCES
  ${BSP_ROOT_PATH}/ST/L4/Src/*.c
  ${BSP_ROOT_PATH}/ST/L4/Src/Legacy/*.c

)


file(GLOB SYSTEM_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/Src/system_stm32l4xx.c
 # ${CMAKE_CURRENT_SOURCE_DIR}/Src/cube_hal_l4.c
 # ${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32l4xx_hal_msp.c
)

file(
    GLOB SENSOR_TILE_SOURCE
    ${BSP_ROOT_PATH}/ST/steval/SensorTile/SensorTile.c
    ${BSP_ROOT_PATH}/ST/steval/SensorTile/SensorTile_env_sensors.c
    ${BSP_ROOT_PATH}/ST/steval/SensorTile/SensorTile_env_sensors_ex.c
    ${BSP_ROOT_PATH}/ST/steval/SensorTile/SensorTile_motion_sensors.c
    ${BSP_ROOT_PATH}/ST/steval/SensorTile/SensorTile_motion_sensors_ex.c
    ${BSP_ROOT_PATH}/ST/steval/SensorTile/SensorTile_sd.c
    ${BSP_ROOT_PATH}/ST/steval/SensorTile/SensorTile_bus.c
    #${BSP_ROOT_PATH}/ST/steval/Components/pcm1774/*.c
    ${BSP_ROOT_PATH}/ST/steval/Components/hts221/*.c
    ${BSP_ROOT_PATH}/ST/steval/Components/lps22hb/*.c
    ${BSP_ROOT_PATH}/ST/steval/Components/lsm6dsm/*.c
    ${BSP_ROOT_PATH}/ST/steval/Components/lsm303agr/*.c
    ${BSP_ROOT_PATH}/ST/steval/Components/stc3115/*.c
    )


set(ASM_SOURCE_FILE  ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32l476xx.s )

message_color(Red ${PROJECT_NAME})
#set(EXECUTABLE "${PROJECT_NAME}.elf")

set(USB_SOURCE_FILE 


${BSP_ROOT_PATH}/ST/usbd/Core/Src/usbd_core.c
${BSP_ROOT_PATH}/ST/usbd/Core/Src/usbd_ctlreq.c
${BSP_ROOT_PATH}/ST/usbd/Core/Src/usbd_ioreq.c

${CMAKE_CURRENT_SOURCE_DIR}/Src/usbd_conf.c
${CMAKE_CURRENT_SOURCE_DIR}/Src/usbd_desc.c
${CMAKE_CURRENT_SOURCE_DIR}/Src/usbd_cdc_interface.c
)

set(FASFS_SIURCE_FILE

${BSP_ROOT_PATH}/ST/FatFs/src/*.c


)


file(GLOB  SOURCE_FILE 

#${CMAKE_CURRENT_SOURCE_DIR}/Src/datalog_application.c
#${CMAKE_CURRENT_SOURCE_DIR}/Src/cube_hal_l4.c
#${CMAKE_CURRENT_SOURCE_DIR}/Src/sd_diskio.c
#${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32l4xx_hal_msp.c
#${CMAKE_CURRENT_SOURCE_DIR}/Src/stm32l4xx_it.c
#${CMAKE_CURRENT_SOURCE_DIR}/Src/syscalls.c
)
set( SOURCE_FILE
    ${HAL_SOURCES}
    ${ASM_SOURCE_FILE}
    ${SYSTEM_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test.c
    

    #${SENSOR_TILE_SOURCE}
    # ${USB_SOURCE_FILE}
    )

add_executable(${PROJECT_NAME}  ${SOURCE_FILE}  )

target_include_directories(


    ${PROJECT_NAME} PUBLIC
    ${BSP_ROOT_PATH}/ST/steval/SensorTile
    ${BSP_ROOT_PATH}/ST/L4/Device/ST/STM32L4xx/Include
    ${BSP_ROOT_PATH}/ST/L4/Inc
    ${BSP_ROOT_PATH}/ST/L4/Inc/Legacy
    ${BSP_ROOT_PATH}/ST/steval/Components/Common/
    ${BSP_ROOT_PATH}/ST/steval/Components/pcm1774/
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${CMSIS_PATH}

    ${BSP_ROOT_PATH}/ST/steval/Components/hts221/
    ${BSP_ROOT_PATH}/ST/steval/Components/lps22hb/
    ${BSP_ROOT_PATH}/ST/steval/Components/lsm6dsm/
    ${BSP_ROOT_PATH}/ST/steval/Components/lsm303agr/
    ${BSP_ROOT_PATH}/ST/steval/Components/stc3115/

    ${CMAKE_CURRENT_SOURCE_DIR}/Inc
    ${BSP_ROOT_PATH}/ST/usbd/Core/Inc
    ${BSP_ROOT_PATH}/ST/usbd/Class/CDC/Inc

    ${BSP_ROOT_PATH}/ST/FatFs/src/



)




target_compile_definitions(${PROJECT_NAME} PUBLIC
    -DDebug
    -DUSE_HAL_DRIVER
    -DSTM32L476xx
    -DUSE_STM32L4XX_NUCLEO
    -DSTM32_SENSORTILE
)


# set CMAKE_C_FLAGS

set(CPU_FLAGS -mthumb -mthumb-interwork -mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16)
set(DEBUG_FLAGS 3)
set(OPTIMIZATION_FLAGS O0)


set(CFLAGS

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
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32L476RGTx_FLASH.ld
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
        m
)


#include jlink for flash



#generate_jlink_basic_cmd("STM32L476RG")
generate_output_file("${PROJECT_NAME}")
generate_jlink_cmd(${PROJECT_NAME}  "STM32L476RG" "${PROJECT_NAME}.bin" "0x08000000")

install_output_file("${PROJECT_NAME}" ${CMAKE_PROJECT_VERSION})







