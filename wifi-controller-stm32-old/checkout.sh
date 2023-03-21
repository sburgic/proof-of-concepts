#!/bin/bash

# Wifi Controller checkout script

wget --no-check-certificate -O ./\source/libs/sl.zip "https://docs.google.com/uc?export=download&id=0B5gpDXfbL8XGN2FNVXliSUJIUzQ"

unzip ./\source/libs/sl.zip -d ./\source/libs/
rm -rf ./\source/libs/sl.zip

