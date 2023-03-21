PROJECT(wifi)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
ENABLE_LANGUAGE(ASM)

# includes for all platforms
SET(INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/application/
    ${CMAKE_CURRENT_SOURCE_DIR}/source/application/include/
)

# includes specific for platform STM32F4
IF(PLATFORM MATCHES "STM32F1")
    SET(INCLUDE_DIRS 
        ${INCLUDE_DIRS}
        source/libs/STM32F1xx_HAL_Driver/Inc/
        source/libs/STM32F1xx_HAL_Driver/Inc/Legacy/
        source/libs/CMSIS/Device/ST/STM32F1xx/Include/
        source/libs/CMSIS/Include/
        source/libs/service_layer/include/
        source/libs/one_wire/include/
    )
ENDIF()

# include sum of includes 
INCLUDE_DIRECTORIES(
    ${INCLUDE_DIRS}
)

# set additional definitions, for example project specific
IF (ADDITIONAL_DEFINITIONS)
    add_definitions(${ADDITIONAL_DEFINITIONS})
ENDIF()

# add subdirectories with CMakeLists.txt files for platform specific
IF(PLATFORM MATCHES "STM32F1")
    # build CMSIS lib
    # add_subdirectory(libs/CMSIS)

    # build STM32F1xx_HAL_Driver lib
    add_subdirectory(source/libs/STM32F1xx_HAL_Driver)

    # build service_layer lib
    add_subdirectory(source/libs/service_layer)

    # build one_wire lib
    add_subdirectory(source/libs/one_wire)
ENDIF()

# add project sources for specific platforms
IF(PLATFORM MATCHES "STM32F1")
    SET(PROJECT_SOURCES
        source/application/src/main.c
        source/application/src/stm32f1xx_it.c
        source/application/src/stm32f1xx_hal_msp.c
        source/application/src/system_stm32f1xx.c
        source/application/src/startup_stm32f100xb.s
        source/application/src/bl_uart.c
        source/application/src/bsp_time.c
        source/application/src/callbacks.c
        source/application/src/ds18b20.c
        source/application/src/bsp_wifi_controller.c
        source/application/src/esp8266.c
        source/application/src/bl_flash.c
        source/application/src/cli.c
        source/application/src/cli_sys.c
        source/application/src/cli_esp8266.c
        source/application/src/lock.c
    )
ENDIF()

# make executable
ADD_EXECUTABLE(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})

# link rules for specific platforms
IF(PLATFORM MATCHES "STM32F1")
	# set linker script
	IF(NOT LINKER_SCRIPT)
		MESSAGE(FATAL_ERROR "No linker script specified. Please specify linker script using LINKER_SCRIPT variable.")
	ENDIF()
	SET_LINKER_SCRIPT(${CMAKE_PROJECT_NAME} ${LINKER_SCRIPT})

	# link exe and libs
	TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} STM32F1xx_HAL_Driver SL_LIB ONE_WIRE_LIB)

	# add make target for hex and bin files
	ADD_HEX_BIN_TARGETS(${CMAKE_PROJECT_NAME})
    
    # add target for size operation
    ADD_CUSTOM_TARGET(size ALL COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME})
	
	# configure and copy scripts from scripts directorium
	# CONFIGURE_FILE(scripts/flash-with-gdb.sh.in ${PROJECT_BINARY_DIR}/flash-with-gdb.sh  @ONLY IMMEDIATE)
	
	# copy all files from scripts dir to build dir
	ADD_CUSTOM_COMMAND(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/scripts ${PROJECT_BINARY_DIR})
ENDIF()
