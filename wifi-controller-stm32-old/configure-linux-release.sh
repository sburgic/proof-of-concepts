#!/bin/bash
# Linux build script (release)

mkdir build-stm32f1-gcc-release
cd build-stm32f1-gcc-release
cmake -DCMAKE_TOOLCHAIN_FILE=../stm32-gcc.cmake -Wno-deprecated -DPLATFORM=STM32F1 -DCORTEX_FAMILY=M3 -DADDITIONAL_DEFINITIONS="-DSTM32F100xB -DUSE_HAL_DRIVER -DARM_MATH_CM3" -DLINKER_SCRIPT=../STM32F100VB_FLASH.ld -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" ..
