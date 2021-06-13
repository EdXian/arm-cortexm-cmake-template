
cmake_minimum_required(VERSION 3.14.0)
project(RTOS_simulation)

if(MINGW OR CYGWIN OR WIN32)
	#ssset(UTIL_SEARCH_CMD where)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -ggdb -g -UUSE_STDIO -std=c99")
elseif(UNIX OR APPLE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -ggdb -g -UUSE_STDIO ")
	#set(UTIL_SEARCH_CMD which)
endif()


#execute_process(
#	COMMAND ${UTIL_SEARCH_CMD} gcc
#	OUTPUT_VARIABLE COMPILER
#        OUTPUT_STRIP_TRAILING_WHITESPACE
#)

#set(CMAKE_C_COMPILER ${COMPILER})


option(BUILD_DSP_LIB ON)
option(BUILD_ALGO_LIB ON)
option(BUILD_DSP_PYTHONWRAPPER ON)

set(THIRD_PARTY_PATH ${CMAKE_SOURCE_DIR}/../thirdparty)
#build dsp library



set(ROOT ${THIRD_PARTY_PATH}/CMSIS_5)
message( STATUS ${ROOT})
set(DSP ${ROOT}/CMSIS/DSP)
list(APPEND CMAKE_MODULE_PATH ${DSP})
add_subdirectory("${DSP}/Source" bin_dsp)

function(compilerSpecificCompileOptions PROJECTNAME ROOT)
endfunction()

#FREE
set(FREERTOS_PATH ${THIRD_PARTY_PATH}/FreeRTOS)
set(ALGO_PATH ${CMAKE_SOURCE_DIR}/../dsptest_rtos/Bio-Detection_SAME_Platform/Bio-Detection_SAM_Platform/algo)


if(MINGW)
file(GLOB RTOS_SOURCE_FILE
    ${FREERTOS_PATH}/Source/portable/MSVC-MingW/port.c
    ${FREERTOS_PATH}/Source/portable/MemMang/heap_4.c
    ${FREERTOS_PATH}/Source/*.c
    ${FREERTOS_PATH}/Source/portable/Common/code_coverage_additions.c
    ${FREERTOS_PATH}/Source/portable/Common/Run-time-stats-utils.c
    )
set(
    RTOS_INCLUDE
    ${CMAKE_SOURCE_DIR}/windows_sim
    ${FREERTOS_PATH}
    ${FREERTOS_PATH}/Source/include
    ${FREERTOS_PATH}/Source/portable/MSVC-MingW
)

elseif(UNIX)

file(GLOB RTOS_SOURCE_FILE
    ${FREERTOS_PATH}/Source/*.c
    ${FREERTOS_PATH}/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c
	${FREERTOS_PATH}/Source/portable/ThirdParty/GCC/Posix/port.c
	${FREERTOS_PATH}/Source/portable/MemMang/heap_4.c	
    )
include_directories(
    RTOS_INCLUDE
    ${CMAKE_SOURCE_DIR}/linux_sim
      ${FREERTOS_PATH}
    ${FREERTOS_PATH}/Source
    ${FREERTOS_PATH}/Source/include
    ${FREERTOS_PATH}/Source/portable/ThirdParty/GCC/Posix
    ${FREERTOS_PATH}/Source/portable/ThirdParty/GCC/Posix/utils
    ${FREERTOS_PATH}/Source/portable/MemMang
    ${FREERTOS_PATH}/Source/portable
    ${ALGO_PATH}
)
endif()


file(GLOB ALGO_SROURCES
    ${ALGO_PATH}/*.c
    )


if(BUILD_ALGO_LIB)
add_library(
    ALGO_LIB STATIC
    ${ALGO_SROURCES}
    ${RTOS_SOURCE_FILE}
)

target_compile_definitions(ALGO_LIB PRIVATE
           -DDEBUG=1
           -DUSE_OS=1
           -D__GNUC_PYTHON__
           -D__GCC_POSIX__=1
           -DPLATFORM=1
           -DprojCOVERAGE_TEST=1
    )
target_include_directories(
        ALGO_LIB PUBLIC
        ${ALGO_PATH}
        ${RTOS_INCLUDE}
    )

target_link_libraries(
    ALGO_LIB
    CMSISDSP
    m
)

add_dependencies(
    ALGO_LIB
    CMSISDSP
    )
endif()


if(MINGW)


set(win_simulation simulation)


add_executable(${win_simulation} ${CMAKE_SOURCE_DIR}/main.c ${RTOS_SOURCE_FILE} ${ALGO_SROURCES}    )

target_include_directories(
    ${win_simulation} PUBLIC
    ${ALGO_PATH}
    ${RTOS_INCLUDE}
)


target_compile_definitions(${win_simulation} PRIVATE
    -DDEBUG=1
    -D__GNUC_PYTHON__
    -D__GCC_POSIX__=1
    -DprojCOVERAGE_TEST=1
    -DUSE_OS=1
    -DPLATFORM=1
    -DprojCOVERAGE_TEST=1
    )

#add_executable()
target_link_libraries(
${win_simulation} PRIVATE
    winMM.Lib
    CMSISDSP
    m
    ALGO_LIB
)

elseif(UNIX)

set(linux_simulation simulation.out)
add_executable(${linux_simulation} ${CMAKE_SOURCE_DIR}/main.c  ${RTOS_INCLUDE} ${RTOS_SOURCE_FILE})

target_compile_definitions(${linux_simulation} PRIVATE
    -DDEBUG=1
    -D__GNUC_PYTHON__
    -D__GCC_POSIX__=1
    -DprojCOVERAGE_TEST=1
    -DUSE_OS=1
    -DPLATFORM=1
    -DprojCOVERAGE_TEST=1
)


#add_executable()
target_link_libraries(
${linux_simulation} PRIVATE
     pthread
     CMSISDSP
         m
     ALGO_LIB
)

endif()

#add subdirectory
add_subdirectory(unit_test)



# build Python wrapper

if(BUILD_DSP_PYTHONWRAPPER)


if(MINGW) #for windows case

execute_process(
  COMMAND python --version
  OUTPUT_VARIABLE PYTHON_VER
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS ${PYTHON_VER})
message(STATUS "add custom target : make dsp_pythonwrapper")
add_custom_target(
dsp_pythonwrapper
  COMMAND  
  cd ${PYTHONWRAPPER_PATH} && ./extend.bat
)

endif()



if(UNIX) #for linux case

find_package(Python3 COMPONENTS Interpreter  Development NumPy)

message(STATUS "Python3 FOUND ${PYTHON_VER}")
message(STATUS "pythonexe ${PYTHON_EXECUTABLE}")

if (Python3_FOUND)
  execute_process(
  COMMAND ${PYTHON_EXECUTABLE} --version
  OUTPUT_VARIABLE PYTHON_VER
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(HAS_PYTHON TRUE)
endif()

if(HAS_PYTHON)
message(STATUS "add custom target : make dsp_pythonwrapper")

set(PYTHONWRAPPER_PATH "${CMAKE_SOURCE_DIR}/PythonWrapper")
message(STATUS "python wrapper location : ${PYTHONWRAPPER_PATH}")
add_custom_target(
dsp_pythonwrapper
  COMMAND  
  cd ${PYTHONWRAPPER_PATH} && ./extend.sh
)

endif(HAS_PYTHON) #HAS_PYTHON

endif(UNIX) #UNIX





endif()












