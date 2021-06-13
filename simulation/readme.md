# simulation setup

This simulation can be built under UBUNTU 16.04 - 18.04

## cmake
 *greater than 3.16.0 

```
git clone https://github.com/Kitware/CMake
cd CMake
sudo ./bootstrap
make
sudo make install
```

## gcc
build gnu c compiler for the system
```
sudo apt-get update
sudo apt-get install build-essential
gcc --version
```

## qt

* qt creator 4.8

```
sudo apt-get install qtcreator
```


### update CMSIS 5 module

```
cd frequency_domain_als
git checkout Ed
//to download the CMSIS module for the simulation
source config.sh
cd project/dsptest/linux_sim
mkdir build &&cd build
#set option HOST=ON
cmake -DHOST=ON..
ccmake ..
make

```

### Build Test

set option flag for the unitest you want to use.

BUILD_PARSER_LIB
BUILD_ALGO_TEST
BUILD_PEAK_TEST
BUILD_SCENARIO_TEST
BUILD_ALGO_LIB
BUILD_SPECTRUM_TEST

## for dsp python wrapper
python version > v3.6
use apt installer or pip installer to install the following packages
1. distutils
2. numpy
3. matplotlib
4. scipy
5. dev

```
sudo apt-get install python3-distutils \
python3-numpy \
python3-matplotlib \
python3-scipy \
python3-dev


pip install "scipy==1.4.1" 
```

### build python wrapper
```
cmake -DBUILD_DSP_PYTHONWRAPPER=ON ..
make python_wrapper

```





