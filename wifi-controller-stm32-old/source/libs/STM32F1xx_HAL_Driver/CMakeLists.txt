
include_directories(
    Inc/
    Inc/Legacy/
)

set(STDPERIPH_SOURCES
    Src/stm32f1xx_hal.c
    Src/stm32f1xx_hal_adc.c
    Src/stm32f1xx_hal_adc_ex.c
    Src/stm32f1xx_hal_can.c
    Src/stm32f1xx_hal_cec.c
    Src/stm32f1xx_hal_cortex.c
    Src/stm32f1xx_hal_crc.c
    Src/stm32f1xx_hal_dac.c
    Src/stm32f1xx_hal_dac_ex.c
    Src/stm32f1xx_hal_dma.c
    Src/stm32f1xx_hal_eth.c
    Src/stm32f1xx_hal_flash.c
    Src/stm32f1xx_hal_flash_ex.c
    Src/stm32f1xx_hal_gpio.c
    Src/stm32f1xx_hal_gpio_ex.c
    Src/stm32f1xx_hal_hcd.c
    Src/stm32f1xx_hal_i2c.c
    Src/stm32f1xx_hal_i2s.c
    Src/stm32f1xx_hal_irda.c
    Src/stm32f1xx_hal_iwdg.c
    Src/stm32f1xx_hal_msp_template.c
    Src/stm32f1xx_hal_nand.c
    Src/stm32f1xx_hal_nor.c
    Src/stm32f1xx_hal_pccard.c
    Src/stm32f1xx_hal_pcd.c
    Src/stm32f1xx_hal_pcd_ex.c
    Src/stm32f1xx_hal_pwr.c
    Src/stm32f1xx_hal_rcc.c
    Src/stm32f1xx_hal_rcc_ex.c
    Src/stm32f1xx_hal_rtc.c
    Src/stm32f1xx_hal_rtc_ex.c
    Src/stm32f1xx_hal_sd.c
    Src/stm32f1xx_hal_smartcard.c
    Src/stm32f1xx_hal_spi.c
    Src/stm32f1xx_hal_spi_ex.c
    Src/stm32f1xx_hal_sram.c
    Src/stm32f1xx_hal_tim.c
    Src/stm32f1xx_hal_tim_ex.c
    Src/stm32f1xx_hal_uart.c
    Src/stm32f1xx_hal_usart.c
    Src/stm32f1xx_hal_wwdg.c
    Src/stm32f1xx_ll_fsmc.c
    Src/stm32f1xx_ll_sdmmc.c
    Src/stm32f1xx_ll_usb.c
)

add_library(STM32F1xx_HAL_Driver ${STDPERIPH_SOURCES})

# SET_TARGET_PROPERTIES(STM32F4xx_HAL_Driver PROPERTIES COMPILE_DEFINITIONS "STM32F429xx;USE_HAL_DRIVER")