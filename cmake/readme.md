## Cmake Modeuls


## Compiler & Toolchain
The following cmake modules define the compiler and toolchain for building environment.
* arm-toolchain.cmake
* win32-Toolchain.cmake
* unix-toolchain.cmake

cmake modules can help you find the compiler automatically and set the binutils path for projects.
  ```
  ${CMAKE_C_COMPILER} = ...
  ${CMAKE_CXX_COMPILE} = ...
  ${CMAKE_ASM_COMPILER} = ...
  ```
Specify toolchain file before building projects.
```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=xxx-toolchain.cmake .   

xxx is the toolchain you selected.
```



## Python Modeuls

include python.cmake before you use the python modules
  ```
  if(Python3_FOUND)

    # do something here

  endif()
  ```

## bsp
bsp.cmake defines serveral variables which can help you develop bsp or application.
```
${BSP_ROOT_PATH}   #path to root
${COMPONENTS_ROOT_PATH} # path to Components root
${THIRDPARTY_ROOT_PATH} # path to third party root
${UITILITY_ROOT_PATH} # path to utility root
${FLASH_ROOT_PATH} # path to flash root
```

for example if you want to include CMSIS DSP in your projects. you can define as follows:

```
include(${CMAKE_MODULE_PATH}/bsp.cmake)
...
set(DSP_ROOT ${THIRDPARTY_ROOT_PATH}/CMSIS_5)
set(DSP ${ROOT}/CMSIS/DSP)
list(APPEND CMAKE_MODULE_PATH ${DSP})
add_subdirectory("${DSP}/Source" bin_dsp)
```



## JLink
For flashing the binary file, we can include jlink modules to our projects.
```
include(${CMAKE_MODULE_PATH}/jlink.cmake)

project("myproject")

#generates binary or hex files
generate_output_file(${PROJECT_NAME})

#generates flash command
generate_jlink_cmd(${PROJECT_NAME} "DEVICE" "xxx.bin" "0x000")

```
