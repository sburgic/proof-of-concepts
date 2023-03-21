## Name
##   Makefile
##
## Purpose
##   Definitions and Rules for VAMeter project
##
## Revision
##   10-Oct-2020 (SSB) [] Initial
##   26-Oct-2020 (SSB) [] Add PCD8544 driver

BASE_DIR := ../base
LIBS_DIR := ../libs

STM_CUBE_LIB_VER  := FW_F1_V1.8.0
STM_CUBE_LIB_ROOT := $(LIBS_DIR)/STM32Cube_$(STM_CUBE_LIB_VER)/Drivers

include $(BASE_DIR)/oshelpers.mk

ifeq ($(HOST_OS),)
    $(error "Host OS not defined!")
endif

ifeq ($(BUILD_TYPE),)
    $(error "Build type not defined!")
endif

ifeq ($(MCU),)
    $(error "MCU not defined!")
else ifeq ($(MCU),STM32F100xB)
    CPU_FAMILY := M3
    LDSCRIPT   := source/STM32F100.ld
else
    $(error "MCU not supported!")
endif

# Get project name from root directory name
PROJECT_NAME := $(notdir $(CURDIR))

# Output directory definitons
BUILD_DIR := build_$(HOST_OS)_$(BUILD_TYPE)
OBJ_DIR   := $(BUILD_DIR)/app/obj
BIN_DIR   := $(BUILD_DIR)/app/bin
DEP_DIR   := $(BUILD_DIR)/app/dep

include $(BASE_DIR)/toolchain.mk

# Source file directories
SRC_ROOT_DIR := source

# List of object files to be compiled
APP_OBJ_LIST := adc.o \
                buffer.o \
                cli.o \
                cli_sys.o \
                display.o \
                flash.o \
                gpio.o \
                interrupt.o \
                main.o \
                pcd8544.o \
                state_machine.o \
                system_init.o \
                tim.o \
                uart.o

SYS_OBJ_LIST := syscalls.o \
                system_stm32f1xx.o \
                startup_stm32f100xb.o

STM_HAL_OBJ_LIST := stm32f1xx_hal.o \
                    stm32f1xx_hal_adc.o \
                    stm32f1xx_hal_adc_ex.o \
                    stm32f1xx_hal_cortex.o \
                    stm32f1xx_hal_dma.o \
                    stm32f1xx_hal_gpio.o \
                    stm32f1xx_hal_pwr.o \
                    stm32f1xx_hal_rcc.o \
                    stm32f1xx_hal_rcc_ex.o \
                    stm32f1xx_hal_spi.o \
                    stm32f1xx_hal_tim.o \
                    stm32f1xx_hal_tim_ex.o \
                    stm32f1xx_hal_uart.o

vpath %.c $(SRC_ROOT_DIR)/application \
          $(SRC_ROOT_DIR)/application/src \
          $(STM_CUBE_LIB_ROOT)/STM32F1xx_HAL_Driver/Src

vpath %.s $(SRC_ROOT_DIR)/application

# Header file directories
CC_INC_DIR  := $(LIBS_DIR) \
               $(SRC_ROOT_DIR)/application/include \
               $(STM_CUBE_LIB_ROOT)/STM32F1xx_HAL_Driver/Inc \
               $(STM_CUBE_LIB_ROOT)/STM32F1xx_HAL_Driver/Inc/Legacy \
               $(STM_CUBE_LIB_ROOT)/CMSIS/Include \
               $(STM_CUBE_LIB_ROOT)/CMSIS/Device/ST/STM32F1xx/Include

# Application specific defines to be forwarded to the command line build
APP_DEFS := -DUSE_HAL_DRIVER \
            -D$(MCU)
            # -D__weak=__attribute__((weak)) \
            # -D__packed=__attribute__((__packed__))


OBJ_LIST := $(addprefix $(OBJ_DIR)/,$(APP_OBJ_LIST))
OBJ_LIST += $(addprefix $(OBJ_DIR)/,$(SYS_OBJ_LIST))

STM_HAL_LIB_OBJ_DIR  := $(BUILD_DIR)/stm/hal/obj
STM_HAL_LIB_OUT_DIR  := $(BUILD_DIR)/stm/hal/lib
STM_HAL_LIB_OBJ_LIST := $(addprefix $(STM_HAL_LIB_OBJ_DIR)/,$(STM_HAL_OBJ_LIST))
STM_HAL_LIB          := stm32_hal.a

# Library directories
CC_LIB_DIR := $(STM_HAL_LIB_OUT_DIR)

CC_INC_FLAG   := -I
CC_INC_PARAMS := $(addprefix $(CC_INC_FLAG),$(CC_INC_DIR))

CC_LINK_INC_FLAG   := -L
CC_LINK_INC_PARAMS := $(addprefix $(CC_LINK_INC_FLAG),$(CC_LIB_DIR))

CC_LINK_FLAG := -l
CC_LINK_LIBS := $(addprefix $(CC_LINK_FLAG),$(basename $(STM_HAL_LIB)))

DEP_FLAGS += -MMD -MP -MF $(DEP_DIR)/$*.d
DEP_FILES := $(addprefix $(DEP_DIR)/, \
             $(addsuffix .d, $(notdir $(basename $(OBJ_LIST)))))

include $(wildcard $(DEP_DIR)/*.d)

#
# Build rules
#

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR) $(DEP_DIR)
	@echo "Compiling $@"
	$(CC) $(CFLAGS) $(DEP_FLAGS) $(APP_DEFS) $(CC_INC_PARAMS) -o $@ -c $<

$(OBJ_DIR)/%.o: %.s | $(OBJ_DIR)
	@echo "Assembling $@"
	$(CC) $(CFLAGS) -o $@ -c $<

$(TARGET_ELF): $(STM_HAL_LIB) $(OBJ_LIST) | $(BIN_DIR)
	@echo "Linking $@"
	$(CC) -o $@ $(OBJ_LIST) $(LDFLAGS) $(CC_LINK_INC_PARAMS) $(CC_LINK_LIBS)

$(TARGET_BIN): $(TARGET_ELF)
	$(OBJCPY) -O binary $< $@

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCPY) -O ihex $< $@

size: $(TARGET_ELF)
	$(SIZE) $(TARGET_ELF)

$(STM_HAL_LIB_OBJ_DIR)/%.o: %.c | $(STM_HAL_LIB_OBJ_DIR)
	@echo "Compiling $@"
	$(CC) $(CFLAGS) $(APP_DEFS) $(CC_INC_PARAMS) -o $@ -c $<

$(STM_HAL_LIB): $(STM_HAL_LIB_OBJ_LIST) | $(STM_HAL_LIB_OUT_DIR)
	@echo "Building library $@"
	$(AR) -cr $(STM_HAL_LIB_OUT_DIR)/lib$@ $^

$(OBJ_DIR):
	$(MD) $@

$(BIN_DIR):
	$(MD) $@

$(DEP_DIR):
	$(MD) $@

$(STM_HAL_LIB_OBJ_DIR):
	$(MD) $@

$(STM_HAL_LIB_OUT_DIR):
	$(MD) $@

flash:
	st-flash erase
	st-flash write $(TARGET_BIN) 0x8000000

#
# Recipes
#

all: $(STM_HAL_LIB) $(TARGET_ELF) $(TARGET_BIN) $(TARGET_HEX) size

clean:
	$(RM) $(OBJ_DIR)
	$(RM) $(BIN_DIR)
	$(RM) $(DEP_DIR)

clean_all:
	$(RM) $(BUILD_DIR)

.PHONY: all clean clean_all
