set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

#list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/)
if(NOT DEFINED CMAKE_MODULE_PATH)
message(STATUS "CMAKE_MODULE_PATH is not defined.")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/)
message(STATUS "CMAKE_MODULE_PATH is defined as ${CMAKE_MODULE_PATH}")
elseif(DEFINED CMAKE_MODULE_PATH)
#message(STATUS "CMAKE_MODULE_PATH is defined.")
#list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/)
endif(NOT DEFINED CMAKE_MODULE_PATH)

if(MINGW OR CYGWIN OR WIN32)
    set(UTIL_SEARCH_CMD where)
elseif(UNIX OR APPLE)
    set(UTIL_SEARCH_CMD which)
endif()


include(utils)
include(bsp)
set(TOOLCHAIN_PREFIX "arm-none-eabi-")

execute_process(
    COMMAND ${UTIL_SEARCH_CMD} "${TOOLCHAIN_PREFIX}gcc"
    OUTPUT_VARIABLE BINUTILS_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

get_filename_component(ARM_TOOLCHAIN_DIR ${BINUTILS_PATH} DIRECTORY)

set(CMAKE_PREFIX_PATH ${ARM_TOOLCHAIN_DIR})
set(BINUTILS_PATH ${ARM_TOOLCHAIN_DIR})
message_color(Green ${CMAKE_PREFIX_PATH})
message_color(Green ${BINUTILS_PATH})

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)


set(CMAKE_OBJCOPY ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_SIZE ${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}size CACHE INTERNAL "size tool")

set(CMAKE_FIND_ROOT_PATH ${BINUTILS_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


function(generate_output_file pjt_nm)

    add_custom_command(
        TARGET ${pjt_nm} 
        POST_BUILD 
        DEPENDS ${pjt_nm}.elf
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${pjt_nm}.elf ${pjt_nm}.hex
    )
    
    add_custom_command(
        
        TARGET ${pjt_nm}
        POST_BUILD 
        DEPENDS ${pjt_nm}.elf
        COMMAND ${CMAKE_OBJCOPY} -O binary ${pjt_nm}.elf ${pjt_nm}.bin
    )
    add_custom_command(
        TARGET ${pjt_nm}
        POST_BUILD  
        DEPENDS ${pjt_nm}.elf
        COMMAND ${CMAKE_OBJDUMP} -h -S -C ${pjt_nm}.elf > ${pjt_nm}.lst
    )
    message_color(Yellow "Generate command: make ${pjt_nm}.hex make ${pjt_nm}.bin make ${pjt_nm}.lst")
endfunction(generate_output_file)


function(install_output_file pjt_nm ver)

    install(TARGETS  ${pjt_nm}
        DESTINATION  ${FLASH_ROOT_PATH}/${pjt_nm}_${ver}/
    )
    set(FLASH_RESOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/${pjt_nm}_flash.jlink
    #    ${BSP_ROOT_PATH}/Atmel/SAME51/atmelice.bat
    #    ${BSP_ROOT_PATH}/Atmel/SAME51/loadcode.bat
        )
    install(FILES ${FLASH_RESOURCES}
        DESTINATION  ${FLASH_ROOT_PATH}/${pjt_nm}_${ver}/
     )

endfunction(install_output_file)
