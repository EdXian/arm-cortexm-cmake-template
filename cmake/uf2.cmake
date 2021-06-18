cmake_minimum_required(VERSION 3.16.0)


include(bsp)
include(utils)

find_package (Python COMPONENTS Interpreter Development)

message_color(Yellow ${Python_VERSION})
function(generate_uf2_file project_name app_start_address)

    message_color(Red "generate ${CMAKE_CURRENT_BINARY_DIR}/${project_name}.uf2")
    message_color(Green "${Python3_EXECUTABLE}")
add_custom_target(
     "${project_name}_uf2"
    #python ${CMAKE_SOURCE_DIR}/scripts/uf2/uf2conv.py -c -b 0x4000 -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${UF2_OUTPUT_FILE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BIN_OUTPUT_FILE}

    COMMAND
     ${Python_EXECUTABLE} ${BOOTLOADER_ROOT_PATH}/uf2-bootloader/lib/uf2/utils/uf2conv.py -f 0x68ed2b88 -b ${app_start_address} -o "${project_name}.uf2" -c "${project_name}.bin"
    #DEPENDS ${project_name}.bin
     )


#${BOOTLOADER_ROOT_PATH}/uf2-bootloader/lib/uf2/utils/uf2conv.py
#${pjt_nm}.bin  -c -b ${app_start_address} -o ${CMAKE_CURRENT_BINARY_DIR}/${project_name}.uf2

#add_custom_target (
#        ${project_name}
#        ${PYTHON_EXECUTABLE} ${BOOTLOADER_ROOT_PATH}/uf2-bootloader/lib/uf2/utils/uf2conv.py -c -b ${app_start_address} -o ${project_name}.uf2 ${CMAKE_CURRENT_BINARY_DIR}/}
#        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${project_name}.bin
#)
endfunction(generate_uf2_file)


