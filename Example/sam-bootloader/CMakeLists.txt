cmake_minimum_required(VERSION 3.16.0)

set(PROJECT_VERSION 1.0.0 CACHE STRING "set the version number for projects")
project(sam-bootloader VERSION ${PROJECT_VERSION}  LANGUAGES C ASM )


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




set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/init_samd21.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/flash_samd21.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/fat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cdc_enumerate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hid.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/images.c
   ${CMAKE_CURRENT_SOURCE_DIR}/src/msc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sam_ba_monitor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/screen.c
  ${CMAKE_CURRENT_SOURCE_DIR}/src/startup_samd21.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/uart_driver.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/usart_sam_ba.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils.c
    )




set(DEPEN_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/
     ${CMAKE_CURRENT_SOURCE_DIR}/boards/zero
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/samd21/samd21a/include
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/samd21/samd21a/include/pio
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/samd21/samd21a/include/component
     ${CMAKE_CURRENT_SOURCE_DIR}/lib/samd21/samd21a/include/instance
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    ${CMSIS_PATH}/

    )

set(CPU_FLAGS -mthumb -mcpu=cortex-m0plus  -g ) #-mfloat-abi=soft -mfpu=fpv4-sp-d16
set(DEBUG_FLAGS 3)
set(OPTIMIZATION_FLAGS O0)

set(WFLAGS
    -Werror -Wall -Wstrict-prototypes
    -Werror-implicit-function-declaration -Wpointer-arith -std=gnu99
    -ffunction-sections -fdata-sections -Wchar-subscripts -Wcomment -Wformat=2
    -Wimplicit-int -Wmain -Wparentheses -Wsequence-point -Wreturn-type -Wswitch
    -Wtrigraphs -Wunused -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wno-undef
    -Wbad-function-cast -Wwrite-strings -Waggregate-return
    -Wformat -Wmissing-format-attribute
    -Wno-deprecated-declarations -Wpacked -Wredundant-decls -Wnested-externs
    -Wlong-long -Wunreachable-code -Wcast-align
    -Wno-missing-braces -Wno-overflow -Wno-shadow -Wno-attributes -Wno-packed -Wno-pointer-sign

    )
set(CFLAGS
    -mthumb
    -mcpu=cortex-m0plus
    -Os
    -g
    -DSAMD21

    -x c -c -pipe -nostdlib
    --param max-inline-insns-single=500
    -fno-strict-aliasing
    -fdata-sections
    -ffunction-sections
    -D__$(CHIP_VARIANT)__



#    -nostdlib
#    -x c
#    -c
#    -std=gnu99
#    ${CPU_FLAGS}
#    -Wall
#    -ffunction-sections
#    -fdata-sections
#    -g${DEBUG_FLAGS}
#    -${OPTIMIZATION_FLAGS}
#    -mlong-calls
#    -DSAMD21
#    -D__SAMD21G18A__
#    -x c -c -pipe -nostdlib
#    --param max-inline-insns-single=500
#    -fno-strict-aliasing
#-specs=nano.specs
#-specs=nosys.specs
##     -flto

##    -mabi=aapcs
#      -nostartfiles
#     -DCONF_DFLL_OVERWRITE_CALIBRATION=0
#    -DCFG_TUSB_MCU=OPT_MCU_SAMD21




    )


set(LDFLAGS
     -T${CMAKE_CURRENT_SOURCE_DIR}/scripts/samd21j18a.ld
    -g${DEBUG_FLAGS}
    ${CPU_FLAGS}
    --specs=nano.specs
    -Wl,--gc-sections
)



add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    )



target_include_directories(
    ${PROJECT_NAME} PUBLIC
    ${DEPEN_DIR}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        -DDEBUG

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

