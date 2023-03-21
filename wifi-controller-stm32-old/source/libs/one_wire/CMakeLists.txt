
include_directories(
    include/
    ../STM32F1xx_HAL_Driver/Inc
    ../STM32F1xx_HAL_Driver/Inc/Legacy
    ../service_layer/include
)

set(ONE_WIRE_SOURCES
    src/one_wire.c
)

add_library(ONE_WIRE_LIB ${ONE_WIRE_SOURCES})

TARGET_LINK_LIBRARIES(ONE_WIRE_LIB SL_LIB)
