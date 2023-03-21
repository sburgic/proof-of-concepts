import sys
import serial

rly_cfg_author  = "Sani Sasa Burgic - sani.sasa.burgic@gmail.com"
rly_cfg_ver     = "1.0.0"
rly_cfg_date    = "15-Nov-2019"

print ""
print "Relay controller tool\r\n"
print "Version :", rly_cfg_ver
print "Date    :", rly_cfg_date
print "Author  :", rly_cfg_author
print ""

com_port  = sys.argv[1]
rly_id    = int(sys.argv[2]) - 1
rly_state = int(sys.argv[3])

com_hdl = serial.Serial(com_port)

buff = chr(0xF0) + chr(rly_id) + chr(rly_state)
com_hdl.write(buff)
