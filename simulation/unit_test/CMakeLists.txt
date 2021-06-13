
message(STATUS "Build the unit test file")
message(STATUS ${ALGO_PATH})
message(STATUS ${RTOS_INCLUDE})

# Unit test file
FILE(GLOB UNIT_TEST_SOURCE_FILE
    ${CMAKE_SOURCE_DIR}/unit_test/*.c
)


foreach(file ${UNIT_TEST_SOURCE_FILE})
    get_filename_component(barefile ${file} NAME_WE)
    message(STATUS testfile->${barefile})
    add_executable(${barefile} ${file})
    target_include_directories(${barefile} PUBLIC 
        ${ALGO_PATH}
        ${RTOS_INCLUDE}
    )
    target_compile_definitions(${barefile} PRIVATE
           -DDEBUG=1
           -DUSE_OS=1
           -D__GNUC_PYTHON__
           -D__GCC_POSIX__=1
           -DPLATFORM=1
           -DprojCOVERAGE_TEST=1
    )
    target_link_libraries(${barefile}
        ALGO_LIB
        CMSISDSP
        m
    )

endforeach()

