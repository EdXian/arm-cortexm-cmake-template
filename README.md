# Cmake template for arm-gcc-none-eabi


## Pre Requirement
* cmake (v3.16.0)
* GNU ARM Toolchain (6.3.1)
* JLink


## IDEs

* Qt
* visual studio code
* atom

## submodule



## Project construction

1. cmake

  this folder consists of Toolchain file for compiler
  one can use the following instruction to build
  ```
  cmake -DCMAKE_TOOLCHAIN_FILE=  (path to project)/cmake/xxx.cmake ..
  ```

2. BSP
  This folder contains several low-level drivers and specific APIs provided by vendors.
  you can find the driver by searching the manufacture of chip.
  Currently, the following MCUs are support by this template
  1. ST
  2. SAME51
  3. rp2040


3. thirdparty
  this folder contains the package provided by thirdparty.
4.
5.
