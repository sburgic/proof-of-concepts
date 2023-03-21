# This is working only on Linux OS (will be reworked for Win support in future)

# connect remote gdbserver on port 3333
tar rem :3333

# reset core and hold in reset
monitor reset halt

# choose and load file
file wifi
load

# add breakpoint
b main.c:100

# continue to breakpoint
c
