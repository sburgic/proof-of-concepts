#!/bin/bash
# Load image in flash

st-flash erase
st-flash write ../build-stm32f1-gcc/wifi.bin 0x8000000
st-flash write ../html/index.html 0x0800F400
st-flash write ../html/login_sent.html 0x0800F000
