# CMake script for STM32F1, most of code reused from OBKO.

INCLUDE(CMakeForceCompiler)

IF(NOT TOOLCHAIN_PREFIX)
     IF(WIN32)
        SET(TOOLCHAIN_PREFIX "C:/gcc-arm-none-eabi")
     ELSE()
        SET(TOOLCHAIN_PREFIX "/opt/gcc-arm-none-eabi")
     ENDIF(WIN32)
     MESSAGE(STATUS "No TOOLCHAIN_PREFIX specified, using default: " ${TOOLCHAIN_PREFIX})
ENDIF()

IF(NOT TARGET_TRIPLET)
    SET(TARGET_TRIPLET "arm-none-eabi")
    MESSAGE(STATUS "No TARGET_TRIPLET specified, using default: " ${TARGET_TRIPLET})
ENDIF()

SET(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
SET(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/include)
SET(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/lib)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR arm)

IF(WIN32)
    # For this settings, compiler path should be added to PATH
    CMAKE_FORCE_C_COMPILER(${TARGET_TRIPLET}-gcc GNU)
    CMAKE_FORCE_CXX_COMPILER(${TARGET_TRIPLET}-g++ GNU)
    SET(CMAKE_ASM_COMPILER ${TARGET_TRIPLET}-gcc)
ELSE()
    # For this settings, compiler path should be added to PATH or set full path manually
    CMAKE_FORCE_C_COMPILER("/opt/gcc-arm-none-eabi/bin/arm-none-eabi-gcc" GNU)
    CMAKE_FORCE_CXX_COMPILER("/opt/gcc-arm-none-eabi/bin/arm-none-eabi-g++" GNU)
    SET(CMAKE_ASM_COMPILER "${/opt/gcc-arm-none-eabi/bin/arm-none-eabi-gcc}")
ENDIF()
    
SET(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-objcopy CACHE INTERNAL "objcopy tool")
SET(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-objdump CACHE INTERNAL "objdump tool")
SET(CMAKE_SIZE ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-size CACHE INTERNAL "objsize tool")

SET(CMAKE_C_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "c compiler flags debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "cxx compiler flags debug")
SET(CMAKE_ASM_FLAGS_DEBUG "-g" CACHE INTERNAL "asm compiler flags debug")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "linker flags debug")

SET(CMAKE_C_FLAGS_RELEASE "-Os" CACHE INTERNAL "c compiler flags release")
SET(CMAKE_CXX_FLAGS_RELEASE "-Os" CACHE INTERNAL "cxx compiler flags release")
SET(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "asm compiler flags release")
SET(CMAKE_EXE_LINKER_FLAGS_RELESE "" CACHE INTERNAL "linker flags release")

# this function add ability to make hex and bin files from target
FUNCTION(ADD_HEX_BIN_TARGETS TARGET)
    IF(EXECUTABLE_OUTPUT_PATH)
      SET(FILENAME "${EXECUTABLE_OUTPUT_PATH}/${TARGET}")
    ELSE()
      SET(FILENAME "${TARGET}")
    ENDIF()
    ADD_CUSTOM_TARGET(${TARGET}.hex DEPENDS ${TARGET} COMMAND ${CMAKE_OBJCOPY} -Oihex ${FILENAME} ${FILENAME}.hex)
    ADD_CUSTOM_TARGET(${TARGET}.bin DEPENDS ${TARGET} COMMAND ${CMAKE_OBJCOPY} -Obinary ${FILENAME} ${FILENAME}.bin)
ENDFUNCTION()

IF(NOT CORTEX_FAMILY)
    MESSAGE(STATUS "No CORTEX_FAMILY specified, using default: M4.")
	SET(CORTEX_FAMILY "M4")
ENDIF()

IF(CORTEX_FAMILY MATCHES "M4")
	MESSAGE(STATUS "Using compiler with cortex m4 flags.")
	# cortex-m4 specific flags
	SET(CMAKE_C_FLAGS "-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Wall -std=gnu99 -mabi=aapcs -fsingle-precision-constant --specs=rdimon.specs -lgcc -lc -lm -lrdimon" CACHE INTERNAL "c compiler flags")
	SET(CMAKE_CXX_FLAGS "-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -Wall -std=c++11 -mabi=aapcs -fsingle-precision-constant" CACHE INTERNAL "cxx compiler flags")
	SET(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -x assembler-with-cpp " CACHE INTERNAL "asm compiler flags")
	# Excluded -nostartfiles from initial code from OBKO
	SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,-Map=output.map -mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mabi=aapcs" CACHE INTERNAL "executable linker flags")
	SET(CMAKE_MODULE_LINKER_FLAGS "-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mabi=aapcs" CACHE INTERNAL "module linker flags")
	SET(CMAKE_SHARED_LINKER_FLAGS "-mthumb -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mabi=aapcs" CACHE INTERNAL "shared linker flags")
ELSE()
	IF(CORTEX_FAMILY MATCHES "M3")
		MESSAGE(STATUS "Using compiler with cortex m3 flags.")
		# cortex-m3 specific flags
		SET(CMAKE_C_FLAGS "-mthumb -mcpu=cortex-m3 -Wall -std=gnu99 -mabi=aapcs -fdata-sections -ffunction-sections" CACHE INTERNAL "c compiler flags")
		SET(CMAKE_CXX_FLAGS "-mthumb -mcpu=cortex-m3 -Wall -std=c++11 -mabi=aapcs" CACHE INTERNAL "cxx compiler flags")
		SET(CMAKE_ASM_FLAGS "-mthumb -mcpu=cortex-m3 -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")

		SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,-Map=output.map -mthumb -mcpu=cortex-m3 -mabi=aapcs -fdata-sections -ffunction-sections" CACHE INTERNAL "executable linker flags")
		SET(CMAKE_MODULE_LINKER_FLAGS "-mthumb -mcpu=cortex-m3 -mabi=aapcs" CACHE INTERNAL "module linker flags")
		SET(CMAKE_SHARED_LINKER_FLAGS "-mthumb -mcpu=cortex-m3 -mabi=aapcs" CACHE INTERNAL "shared linker flags")
	ELSE()
		MESSAGE(FATAL_ERROR "Invalid/unsupported cortex family: ${CORTEX_FAMILY}")
	ENDIF()
ENDIF()

# this function sets linker script
FUNCTION(SET_LINKER_SCRIPT TARGET LINKER_SCRIPT)
	GET_TARGET_PROPERTY(TARGET_LD_FLAGS ${TARGET} LINK_FLAGS)
	IF(TARGET_LD_FLAGS)
		SET(TARGET_LD_FLAGS "-T${LINKER_SCRIPT} ${TARGET_LD_FLAGS}")
	ELSE()
		SET(TARGET_LD_FLAGS "-T${LINKER_SCRIPT}")
	ENDIF()
	SET_TARGET_PROPERTIES(${TARGET} PROPERTIES LINK_FLAGS ${TARGET_LD_FLAGS})
ENDFUNCTION()

# this function sets additional compiler definitions, for example project specific defines
# multiple definitions have to be separated by ;
FUNCTION(SET_ADDITIONAL_DEFINITIONS TARGET ADDITIONAL_DEFINITIONS)
	MESSAGE(STATUS "${ADDITIONAL_DEFINITIONS}")
    GET_TARGET_PROPERTY(TARGET_DEFS ${TARGET} COMPILE_DEFINITIONS)
    IF(TARGET_DEFS)
        SET(TARGET_DEFS "${ADDITIONAL_DEFINITIONS};${TARGET_DEFS}")
    ELSE()
        SET(TARGET_DEFS "${ADDITIONAL_DEFINITIONS}")
    ENDIF()
	MESSAGE(STATUS "${TARGET_DEFS}")
    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES COMPILE_DEFINITIONS "${TARGET_DEFS}")
ENDFUNCTION()

# this function is specific for STM32F4 and sets HSE_VALUE
FUNCTION(STM32_SET_HSE_VALUE TARGET STM32_HSE_VALUE)   
    GET_TARGET_PROPERTY(TARGET_DEFS ${TARGET} COMPILE_DEFINITIONS)
    IF(TARGET_DEFS)
        SET(TARGET_DEFS "HSE_VALUE=${STM32_HSE_VALUE};${TARGET_DEFS}")
    ELSE()
        SET(TARGET_DEFS "HSE_VALUE=${STM32_HSE_VALUE}")
    ENDIF()
	MESSAGE(STATUS "${TARGET_DEFS}")
    SET_TARGET_PROPERTIES(${TARGET} PROPERTIES COMPILE_DEFINITIONS "${TARGET_DEFS}")
ENDFUNCTION()