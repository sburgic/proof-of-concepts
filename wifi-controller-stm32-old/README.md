In order to build project following steps are required:

1. Run cmake configuration script
    1. configure-windows.bat - Windows host, debug version
    2. configure-windows-release.bat - Windows host, release version
    3. configure-linux.sh - Linux host, debug version
    4. configure-linux-release.sh - Linux host, release version
    
2. Go to build-stm32f1-gcc folder ( if it has not already been automatically done from configure script )
   and type make wifi.bin ( or make wifi.bin size - to obtain image size )
   
In order to flash image run load_image_to_flash.sh script from build-stm32f1-gcc folder ( This script can be executed only on Linux )
To flash image using Windows host use STM32 ST-LINK Utility

Tools:
    gcc-arm-none-eabi compiler
    texane st-util for linux
    cmake

Manuals and datasheets in docs folder

