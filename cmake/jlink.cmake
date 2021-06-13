cmake_minimum_required(VERSION 3.16.0)
include(${CMAKE_CURRENT_LIST_DIR}/utils.cmake)
find_program(JLINK_CMD 
    NAMES Jlink.exe Jlink JLinkExe
    PATHS 
        "C:/Program Files (x86)/SEGGER/JLink"
        "D:/Program Files (x86)/SEGGER/JLink"
)

if(JLINK_CMD)
    message(STATUS "Find Jlink command")
else()
    message(FATAL_ERROR "Jlink command not found")
endif()

#configure_file(${CMAKE_CURRENT_LIST_DIR}/flash.in ${CMAKE_CURRENT_BINARY_DIR}/flash.jlink)
configure_file(${CMAKE_CURRENT_LIST_DIR}/erase.in ${CMAKE_CURRENT_BINARY_DIR}/erase.jlink)
configure_file(${CMAKE_CURRENT_LIST_DIR}/reset.in ${CMAKE_CURRENT_BINARY_DIR}/reset.jlink)

#add_custom_target(flash
#    COMMAND ${JLINK_CMD} ${JLINK_IP_ARG} -device ${JLINK_DEVICE} -speed 4000 -if SWD -CommanderScript ${CMAKE_CURRENT_BINARY_DIR}/flash.jlink
#    DEPENDS ${PROJECT_NAME}.bin
#)
function(generate_jlink_basic_cmd JLINK_DEVICE)
    add_custom_target(erase
        COMMAND ${JLINK_CMD} ${JLINK_IP_ARG} -device ${JLINK_DEVICE} -speed 4000 -if SWD -CommanderScript ${CMAKE_CURRENT_BINARY_DIR}/erase.jlink
    )
    add_custom_target(reset
        COMMAND ${JLINK_CMD} ${JLINK_IP_ARG} -device ${JLINK_DEVICE} -speed 4000 -if SWD -CommanderScript ${CMAKE_CURRENT_BINARY_DIR}/reset.jlink
    )
endfunction(generate_jlink_basic_cmd)


set(JLINK_CONFIG_FILE_PATH ${CMAKE_CURRENT_LIST_DIR})

function(generate_jlink_cmd project_name JLINK_DEVICE BINARY FLASH_ORIGIN)

    set(JLINK_NAME ${project_name}_flash.jlink)
    configure_file(${JLINK_CONFIG_FILE_PATH}/flash.in ${CMAKE_CURRENT_BINARY_DIR}/${project_name}_flash.jlink)
    add_custom_target(${project_name}_flash
        COMMAND ${JLINK_CMD} ${JLINK_IP_ARG} -device ${JLINK_DEVICE} -speed 4000 -if SWD -CommanderScript ${CMAKE_CURRENT_BINARY_DIR}/${project_name}_flash.jlink
        DEPENDS ${project_name}
    )
    message_color(Yellow "Genearate jlink cmd with args: ${project_name} ${JLINK_DEVICE} ${BINARY} ${FLASH_ORIGIN}")

endfunction(generate_jlink_cmd)
