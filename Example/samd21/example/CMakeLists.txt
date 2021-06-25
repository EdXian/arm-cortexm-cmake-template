cmake_minimum_required(VERSION 3.16.0)

FILE(GLOB TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
message_color(Red test_project)

foreach(file ${TEST_FILES})
    get_filename_component(barefile ${file} NAME_WE)
    message_color(Yellow testfile->${barefile})
    add_executable(${barefile} ${file} ${SOURCE_FILES})
    target_include_directories(${barefile} PUBLIC ${DEPEN_DIR})

    target_compile_definitions(${barefile} PUBLIC
        -DDEBUG
        -D__SAMD21G18A__
        )

    target_compile_options(${barefile} PUBLIC
        ${CFLAGS}
        )


    target_link_options(${barefile} PUBLIC
        ${LDFLAGS} 
        -Wl,-Map=${barefile}.map
        )
    target_link_libraries(${barefile} PUBLIC 
        
        m
    )
    # Generate the hex and bin file
    generate_output_file(${barefile})
    generate_jlink_cmd(${barefile}  "ATSAMD21G18A" "${barefile}.bin" "0x00000000")



endforeach()










