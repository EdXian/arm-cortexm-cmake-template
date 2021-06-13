@echo Configure the makefile
cmake -S . -B ./build -G"Ninja" -DCMAKE_TOOLCHAIN_FILE=../cmake/arm-toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
@echo ===================== completed =====================
